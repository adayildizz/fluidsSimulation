#version 330 core
in vec2 vUV;
out float oDepth;

uniform sampler2D uSrc;      // eye-space depth source
uniform vec2  uInvTexSize;   // (1/w, 1/h)
uniform vec2  uDir;          // (1,0) for horizontal, (0,1) for vertical
uniform int   uRadius;       // e.g. 12
uniform float uSigmaS;       // spatial sigma in pixels, e.g. 6.0
uniform float uSigmaR;       // range sigma in depth units (eye-space), e.g. 0.08

void main(){
    float center = texture(uSrc, vUV).r;

    // If center is our "no-surface" sentinel, just keep it
    if (center > 0.5e9) { oDepth = center; return; }

    float sum = 0.0;
    float wsum = 0.0;

    // Precompute spatial coefficient: exp( - (i^2)/(2*sigmaS^2) )
    float twoSigmaS2 = 2.0 * uSigmaS * uSigmaS;
    float twoSigmaR2 = 2.0 * uSigmaR * uSigmaR;

    for (int i = -uRadius; i <= uRadius; ++i) {
        vec2  uv = vUV + uDir * float(i) * uInvTexSize;
        float d  = texture(uSrc, uv).r;
        // Skip "no-surface" samples
        if (d > 0.5e9) continue;

        float ds = float(i);
        float wSpatial = exp(-(ds*ds) / twoSigmaS2);
        float dr = d - center;
        float wRange  = exp(-(dr*dr) / twoSigmaR2);

        float w = wSpatial * wRange;
        sum  += d * w;
        wsum += w;
    }

    // Include the center sample explicitly
    sum  += center;
    wsum += 1.0;

    oDepth = (wsum > 0.0) ? sum / wsum : center;
}
