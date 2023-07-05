#define PI 3.1415926536
#define LINE_INTENSITY .5
#define GLOW_INTENSITY 1.3
#define LIGHT_YELLOW vec3(0.98, 0.72, 0.25)
#define DARK_YELLOW vec3(0.90, 0.35, 0.125)

/* TODOs:
 * - implement more shapes (tri-, penta-, hexa-, octagon)
 * - use a noise/something less uniform for the color changes
 * - use a noise/non-static value to controll the intensity of glow
 * - maybe make the layers pulse?
 * - reduce brightness blow-out of bright colors
 */

vec2 rotateUv(vec2 uv, float rotation){
    vec2 cosVal = uv * cos(rotation);
    vec2 sinVal = uv * sin(rotation);
    return vec2(cosVal.x +sinVal.y, cosVal.y -sinVal.x);
}

vec2 remapRotateUv(vec2 uv, vec2 newOrigin, float rotation){
    vec2 newPoint = rotateUv(newOrigin, rotation);
    vec2 movingUv = uv -newPoint;
    return movingUv;
}

float glowyOutline(float sdfValue){
    return pow(.01 *LINE_INTENSITY /abs(sdfValue), 1. /GLOW_INTENSITY);
}

vec3 runeCircle(vec2 point, float size, vec3 color){
    float sdf = length(point) -size;
    return color *glowyOutline(sdf);
}

vec3 runeSquare(vec2 point, float size, vec3 color){
    vec2 dist = abs(point) -vec2(size);
    float outerDistance = length(max(dist, .0));
    float innerDistance = min(max(dist.x, dist.y), .0);
    float sdf = outerDistance +innerDistance;
    return color *glowyOutline(sdf);
}

vec3 glowySeal(vec2 uv){
    float piFourth = PI /4.;
    vec3 col = mix(LIGHT_YELLOW, DARK_YELLOW, sin(iTime) *.5 +.5);
    
    vec3 finalColor = vec3(.0);
    finalColor += runeSquare(rotateUv(uv, iTime), .6, col);           // outer square
    finalColor += runeSquare(rotateUv(uv, iTime +piFourth), .6, col); // outer square (offset)
    finalColor += runeSquare(rotateUv(uv, -iTime), .35, col);         // inner square
    finalColor += runeCircle(uv, .7, col);                            // outer circle
    finalColor += runeCircle(uv, .42, col);                           // inner circle
    for(float i = .0; i < 8.; i++){                                   // small circles on the outside
        finalColor += runeCircle(remapRotateUv(uv, vec2(.6), -iTime -piFourth *i), .15, col);
    }
                    
    return finalColor;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ){
    vec2 uv = fragCoord/iResolution.xy *2. -1.;
    uv.x *= iResolution.x /iResolution.y;
    fragColor = vec4(glowySeal(uv), 1.0);
}