#define PI 3.1415926536
#define LINE_INTENSITY .5
#define GLOW_INTENSITY 1.1
#define COLOR_CHANGE_SPEED .1

/* TODOs:
 * - implement more shapes (tri-, penta-, hexa-, octagon)
 * - reduce brightness blow-out of bright colors
 */

float noise21(vec2 p){
    return fract(sin(p.x*100.+p.y*8943.)*5647.);
}

float smoothNoise(vec2 uv){
    vec2 localUv = smoothstep(.0, 1., fract(uv));
    vec2 id = floor(uv);

    float bottomLeft = noise21(id);
    float bottomRight = noise21(id+vec2(1.,.0));
    float bottom = mix(bottomLeft, bottomRight, localUv.x);

    float topLeft = noise21(id+vec2(.0,1.));
    float topRight = noise21(id+vec2(1.,1.));
    float top = mix(topLeft, topRight, localUv.x);

    return mix(bottom, top, localUv.y);
}

float smoothNoiseLayers(vec2 uv, float amountOfLayers){
    float result = .0;
    float sumOfAmplitudes = .0;
    float frequency = 4.;
    float amplitude = 1.;
    for(float i = .0; i < amountOfLayers; i++){
        result += smoothNoise(uv*frequency)*amplitude;
        sumOfAmplitudes += amplitude;
        frequency *= 2.;
        amplitude /= 2.;
    }
    return result/sumOfAmplitudes;
}

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

float glowyOutline(float sdfValue, vec2 point){
    float noiseOffset = .5-smoothNoiseLayers(point +(iTime *.2), 5.);
    sdfValue += noiseOffset * .02;
    noiseOffset *= .1;
    return pow(.01 *(LINE_INTENSITY +noiseOffset) /abs(sdfValue), 1. /(GLOW_INTENSITY +noiseOffset));
}

vec3 runeCircle(vec2 point, float size, vec3 color){
    float sdf = length(point) -size;
    return color *glowyOutline(sdf, point);
}

vec3 runeSquare(vec2 point, float size, vec3 color){
    vec2 dist = abs(point) -vec2(size);
    float outerDistance = length(max(dist, .0));
    float innerDistance = min(max(dist.x, dist.y), .0);
    float sdf = outerDistance +innerDistance;
    return color *glowyOutline(sdf, point);
}

vec3 getColor(){
    return .5 +.5 *cos(iTime *COLOR_CHANGE_SPEED +vec3(0, 2, 4));
}

vec3 getComplementaryColor(){
    vec3 color = getColor();
    float minVal = min(color.r, min(color.g, color.b));
    float maxVal = max(color.r, max(color.g, color.b));
    return vec3(maxVal - minVal)-color;
}

vec3 glowySeal(vec2 uv){
    float piFourth = PI /4.;
    
    float randomBrightnessReduction = .9 +smoothNoiseLayers(uv +iTime, 5.) *.1;
    vec3 col = getColor() * randomBrightnessReduction;
    vec3 inverseCol = getComplementaryColor() * randomBrightnessReduction;
    
    vec3 finalColor = vec3(.0);
    finalColor += runeSquare(rotateUv(uv, iTime), .6, col);           // outer square
    finalColor += runeSquare(rotateUv(uv, iTime +piFourth), .6, col); // outer square (offset)
    finalColor += runeSquare(rotateUv(uv, -iTime), .35, inverseCol);  // inner square
    finalColor += runeCircle(uv, .7, col);                            // outer circle
    finalColor += runeCircle(uv, .42, inverseCol);                    // inner circle
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
