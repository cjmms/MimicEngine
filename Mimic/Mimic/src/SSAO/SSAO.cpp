#include "SSAO.h"

#include <random>
#include "../Core/Camera.h"
#include "../UI_Manager.h"
#include "../ResourceManager.h"

extern UI_Manager UI_Mgr;
extern Camera camera;

SSAO::SSAO(unsigned int KernelSize, unsigned int NoiseTextureLength)
    :KernelSize(KernelSize), NoiseTextureLength(NoiseTextureLength),
    SSAOshader("res/Shaders/SSAO/SSAO.shader")
{
    ComputeSampleKernel();      // generate sample kernel

    FillNoise();                // generate random noise 
    ComputeNoiseTexture();      // write noise into a texture

    FrameBufferSetup(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());         // setup SSAO FBO
}




void SSAO::ComputeSampleKernel()
{
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;

    for (unsigned int i = 0; i < KernelSize; ++i)
    {
        glm::vec3 sample(                               // random direction
            randomFloats(generator) * 2.0 - 1.0,        // range from -1 to 1
            randomFloats(generator) * 2.0 - 1.0,        // range from -1 to 1
            randomFloats(generator)                     // range from 0 to 1
        );

        sample = glm::normalize(sample);
        sample *= randomFloats(generator);              // random weight
        sample *= WeightToSample(i);                    // change the distribution, makeing sample closer to origin
        ssaoKernel.push_back(sample);
    }
}


void SSAO::FillNoise()
{
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;

    for (unsigned int i = 0; i < NoiseTextureLength * NoiseTextureLength; ++i)
    {
        glm::vec3 sample(                               // random direction
            randomFloats(generator) * 2.0 - 1.0,        // range from -1 to 1
            randomFloats(generator) * 2.0 - 1.0,        // range from -1 to 1
            randomFloats(generator)                     // range from 0 to 1
        );

        ssaoNoise.push_back(sample);
    }
}


void SSAO::ComputeNoiseTexture()
{
    glGenTextures(1, &NoiseTexture);
    glBindTexture(GL_TEXTURE_2D, NoiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, NoiseTextureLength, NoiseTextureLength, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // repeat the noise pattern
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // repeat the noise pattern
}




void SSAO::FrameBufferSetup(unsigned int width, unsigned int height)
{
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

    // single grayscale value is enough, that why there is only red channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);       //unbind
}






void SSAO::RenderSSAO(unsigned int gPosition, unsigned int gNormal)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind G-Buffer, bind noiseTexture
    SSAOshader.setTexture("gPosition", gPosition);
    SSAOshader.setTexture("gNormal", gNormal);
    SSAOshader.setTexture("NoiseTexture", NoiseTexture);

    SendKernelSamplesToShader();

    SSAOshader.setMat4("view", camera.getViewMatrix());
    SSAOshader.setMat4("projection", camera.getProjectionMatrix());

    Quad::Quad().Draw(SSAOshader);              // render SSAO into a quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void SSAO::SendKernelSamplesToShader()
{
    for(unsigned int i = 0; i < KernelSize; ++i)
        SSAOshader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
}
