#include "SSAO.h"

#include <random>


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

    for (unsigned int i = 0; i < NumberOfNoise; ++i)
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // repeat the noise pattern
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // repeat the noise pattern
}