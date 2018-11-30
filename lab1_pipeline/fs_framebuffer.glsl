#version 430 core
// Image processing kernel from here:
// http://haishibai.blogspot.com/2009/09/image-processing-c-tutorial-4-gaussian.html

in vec2 TexCoords;
out vec4 color;

uniform float width;			// Screen width
uniform float height;			// Screen Height
uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;

void main()
{
	// Total value of RGB
	vec3 total = vec3(0.0);

	// Blur amount in pixels
	//float blurAmount = 3.0;

	// Amount of blur - change this value for more or less;
	//float blur = blurAmount/width;

	// Blur in x
	//float hstep = 1.0; float vstep = 0.0;
	//total += texture(screenTexture, vec2(TexCoords.x - 4.0*blur*hstep, TexCoords.y - 4.0*blur*vstep)).rgb * 0.0162162162;
	//total += texture(screenTexture, vec2(TexCoords.x - 3.0*blur*hstep, TexCoords.y - 3.0*blur*vstep)).rgb * 0.0540540541;
	//total += texture(screenTexture, vec2(TexCoords.x - 2.0*blur*hstep, TexCoords.y - 2.0*blur*vstep)).rgb * 0.1216216216;
	//total += texture(screenTexture, vec2(TexCoords.x - 1.0*blur*hstep, TexCoords.y - 1.0*blur*vstep)).rgb * 0.1945945946;
	//total += texture(screenTexture, TexCoords).rgb * 0.2270270270;
	//total += texture(screenTexture, vec2(TexCoords.x + 1.0*blur*hstep, TexCoords.y + 1.0*blur*vstep)).rgb * 0.1945945946;
	//total += texture(screenTexture, vec2(TexCoords.x + 2.0*blur*hstep, TexCoords.y + 2.0*blur*vstep)).rgb * 0.1216216216;
	//total += texture(screenTexture, vec2(TexCoords.x + 3.0*blur*hstep, TexCoords.y + 3.0*blur*vstep)).rgb * 0.0540540541;
	//total += texture(screenTexture, vec2(TexCoords.x + 4.0*blur*hstep, TexCoords.y + 4.0*blur*vstep)).rgb * 0.0162162162;

	// Blur in y
	 //hstep = 0.0;  vstep = 1.0;
	//total += texture(screenTexture, vec2(TexCoords.x - 4.0*blur*hstep, TexCoords.y - 4.0*blur*vstep)).rgb * 0.0162162162;
	//total += texture(screenTexture, vec2(TexCoords.x - 3.0*blur*hstep, TexCoords.y - 3.0*blur*vstep)).rgb * 0.0540540541;
	//total += texture(screenTexture, vec2(TexCoords.x - 2.0*blur*hstep, TexCoords.y - 2.0*blur*vstep)).rgb * 0.1216216216;
	//total += texture(screenTexture, vec2(TexCoords.x - 1.0*blur*hstep, TexCoords.y - 1.0*blur*vstep)).rgb * 0.1945945946;
	//total += texture(screenTexture, TexCoords).rgb * 0.2270270270;
	//total += texture(screenTexture, vec2(TexCoords.x + 1.0*blur*hstep, TexCoords.y + 1.0*blur*vstep)).rgb * 0.1945945946;
	//total += texture(screenTexture, vec2(TexCoords.x + 2.0*blur*hstep, TexCoords.y + 2.0*blur*vstep)).rgb * 0.1216216216;
	//total += texture(screenTexture, vec2(TexCoords.x + 3.0*blur*hstep, TexCoords.y + 3.0*blur*vstep)).rgb * 0.0540540541;
//	total += texture(screenTexture, vec2(TexCoords.x + 4.0*blur*hstep, TexCoords.y + 4.0*blur*vstep)).rgb * 0.0162162162;

    //total = texture

	//color = vec4(total/2.0, 1.0);

	// Simple example to make everything grayscale...
	 //float average = (color.r + color.g + color.b) / 3.0;
     //color = vec4(average, average, average, 1.0);
     //working
     //color = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);


    vec2 offsets[9] = vec2[](
        vec2(-offset, offset), // top-left
        vec2(0.0f, offset), // top-center
        vec2(offset, offset), // top-right
        vec2(-offset, 0.0f),   // center-left
        vec2(0.0f, 0.0f),   // center-center
        vec2(offset, 0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0.0f, -offset), // bottom-center
        vec2(offset, -offset)  // bottom-right    
        );

    float kernel[9] = float[](
        0, -1, 0,
        -1, 4, -1,
        0, -1, 0
        );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 laplacian = vec3(0.0);
    for (int i = 0; i < 9; i++)
        laplacian += sampleTex[i] * kernel[i];


    color = texture(screenTexture, TexCoords);
    float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = vec4(average +0.5*laplacian.x, average + 0.5*laplacian.y, average + 0.5*laplacian.z, 1.0);
    
    //unsharp_masking = vec3(laplacian.x - average, laplacian.y - average, laplacian.z - average);
    //color = vec4(unsharp_masking, 1.0);

}
