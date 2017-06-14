#include "opengl/header.h"
#include "wowScene.h"
#include "shaders.h"
#include "shader/ShaderRuntimeData.h"

#include <iostream>


ShaderRuntimeData * WoWScene::compileShader(std::string shaderName,
        std::string vertShaderString,
        std::string fragmentShaderString,
        std::string *vertExtraDefStringsExtern, std::string *fragExtraDefStringsExtern) {

    std::string vertExtraDefStrings = (vertExtraDefStringsExtern != nullptr) ? *vertExtraDefStringsExtern : "";
    std::string fragExtraDefStrings = (fragExtraDefStringsExtern != nullptr) ? *fragExtraDefStringsExtern : "";


    if (fragExtraDefStringsExtern == nullptr) {
        fragExtraDefStrings = "";
    }

    if (m_enable) {
        vertExtraDefStrings = "#define ENABLE_DEFERRED 1\r\n"
                "#define precision\n"
                "#define lowp\n"
                "#define mediump\n"
                "#define highp\n";
        fragExtraDefStrings = "#define ENABLE_DEFERRED 1\r\n"
                "#define precision\n"
                "#define lowp\n"
                "#define mediump\n"
                "#define highp\n";
    }

    GLint maxVertexUniforms;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexUniforms);
    int maxMatrixUniforms = (maxVertexUniforms / 4) - 6;

    vertExtraDefStrings = vertExtraDefStrings + "#define MAX_MATRIX_NUM "+std::to_string(maxMatrixUniforms)+"\r\n"+"#define COMPILING_VS 1\r\n ";
    fragExtraDefStrings = fragExtraDefStrings + "#define COMPILING_FS 1\r\n";

    vertShaderString = trimmed(vertShaderString.insert(
        vertShaderString.find("\n",vertShaderString.find("#version", 0)+1)+1,
        vertExtraDefStrings));

    fragmentShaderString = trimmed(fragmentShaderString.insert(
            fragmentShaderString.find("\n",fragmentShaderString.find("#version", 0)+1)+1,
            fragExtraDefStrings));

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderConst = (const GLchar *)vertShaderString.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderConst, 0);
    glCompileShader(vertexShader);

    GLint success = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        // Something went wrong during compilation; get the error
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        std::cout << "\ncould not compile vertex shader "<<shaderName<<":" << std::endl
                  << vertexShaderConst << std::endl << std::endl
                  << "error: "<<std::string(infoLog.begin(),infoLog.end())<< std::endl <<std::flush;

        throw "" ;
    }

    /* 1.2 Compile fragment shader */
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderConst = (const GLchar *) fragmentShaderString.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderConst, 0);
    glCompileShader(fragmentShader);

    // Check if it compiled
    success = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // Something went wrong during compilation; get the error
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        std::cout << "\ncould not compile fragment shader "<<shaderName<<":" << std::endl
            << fragmentShaderConst << std::endl << std::endl
            << "error: "<<std::string(infoLog.begin(),infoLog.end())<< std::endl <<std::flush;

        throw "" ;
    }

    /* 1.3 Link the program */
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    const shaderDefinition *shaderDefinition1 = getShaderDef(shaderName.c_str());
    for (int i = 0; i < shaderDefinition1->attributesNum; i++) {
        glBindAttribLocation(program, shaderDefinition1->attributes[i].number, shaderDefinition1->attributes[i].variableName);
    }

    // link the program.
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        throw "could not compile shader:" ;
    }

    //Get Uniforms
    ShaderRuntimeData *data = new ShaderRuntimeData();
    data->setProgram(program);

    GLint count;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    printf("Active Uniforms: %d\n", count);
    for (GLint i = 0; i < count; i++)
    {
        const GLsizei bufSize = 32; // maximum name length
        GLchar name[bufSize]; // variable name in GLSL
        GLsizei length; // name length
        GLint size; // size of the variable
        GLenum type; // type of the variable (float, vec3 or mat4, etc)

        glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);

        data->setUnf(std::string(name), i);
        printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
    }

    return data;
}
void WoWScene::initShaders() {
    const std::string textureCompositionShader = getShaderDef("textureCompositionShader")->shaderString;
    this->textureCompositionShader = this->compileShader("textureCompositionShader", textureCompositionShader, textureCompositionShader);

    const  std::string renderFrameShader = getShaderDef("renderFrameBufferShader")->shaderString;
    this->renderFrameShader        = this->compileShader("renderFrameBufferShader", renderFrameShader, renderFrameShader);

    const  std::string drawDepthBuffer = getShaderDef("drawDepthShader")->shaderString;
    this->drawDepthBuffer          = this->compileShader("drawDepthShader", drawDepthBuffer, drawDepthBuffer);

    const  std::string readDepthBuffer = getShaderDef("readDepthBufferShader")->shaderString;
    this->readDepthBuffer          = this->compileShader("readDepthBufferShader", readDepthBuffer, readDepthBuffer);

    const  std::string wmoShader = getShaderDef("wmoShader")->shaderString;
    this->wmoShader                = this->compileShader("wmoShader", wmoShader, wmoShader);

    const  std::string m2Shader = getShaderDef("m2Shader")->shaderString;
    this->m2Shader                 = this->compileShader("m2Shader", m2Shader, m2Shader);
    this->m2InstancingShader       = this->compileShader("m2Shader", m2Shader, m2Shader,
                                                         new std::string("#define INSTANCED 1\r\n "),
                                                         new std::string("#define INSTANCED 1\r\n "));

    const  std::string bbShader = getShaderDef("m2Shader")->shaderString;
    this->bbShader                 = this->compileShader("drawBBShader", bbShader, bbShader);

    const  std::string adtShader = getShaderDef("adtShader")->shaderString;
    this->adtShader                = this->compileShader("adtShader", adtShader, adtShader);

    const  std::string drawPortalShader = getShaderDef("drawPortalShader")->shaderString;
    this->drawPortalShader         = this->compileShader("drawPortalShader", drawPortalShader, drawPortalShader);

    const  std::string drawFrustumShader = getShaderDef("drawFrustumShader")->shaderString;
    this->drawFrustumShader        = this->compileShader("drawFrustumShader", drawFrustumShader, drawFrustumShader);

    const  std::string drawLinesShader = getShaderDef("drawLinesShader")->shaderString;
    this->drawLinesShader          = this->compileShader("drawLinesShader", drawLinesShader, drawLinesShader);
}

void WoWScene::initGlContext() {

}
void WoWScene::createBlackPixelTexture() {

}
void WoWScene::initAnisotropicExt() {

}
void WoWScene::initArrayInstancedExt() {

}
void WoWScene::initBoxVBO() {

}
void WoWScene::initCaches() {

}
void WoWScene::initCamera() {

}
void WoWScene::initCompressedTextureS3tcExt() {

}
void WoWScene::initDeferredRendering() {

}
void WoWScene::initDepthTextureExt() {

}
void WoWScene::initDrawBuffers(int frameBuffer) {

}

void WoWScene::initVertexArrayObjectExt() {

}

void WoWScene::initRenderBuffers() {

}

void WoWScene::initSceneApi() {

}

void WoWScene::initSceneGraph() {

}

void WoWScene::initTextureCompVBO() {

}

WoWScene::WoWScene() {
    constexpr const shaderDefinition *definition = getShaderDef("adtShader");
//    constexpr const int attributeIndex = getShaderAttribute("m2Shader", "aNormal");
//    constexpr const int attributeIndex = +m2Shader::Attribute::aNormal;
//    constexpr const shaderDefinition *definition = getShaderDef("readDepthBuffer");
//    std::cout << "aHeight = " << definition->shaderString<< std::flush;
    std::cout << "aNormal = " << +m2Shader::Attribute::aNormal << std::flush;
    this->initShaders();
}
/* Shaders stuff */
/*
void WoWScene::activateRenderFrameShader () {
        glUseProgram(this->renderFrameShader->getProgram());
        glActiveTexture(GL_TEXTURE0);

        glDisableVertexAttribArray(1);

        glUniform2fv(this->renderFrameShader->getUnf("uResolution"), new Float32Array([this.canvas.width, this.canvas.height]))

        glUniform1i(this->renderFrameShader->getUnf("u_sampler"), 0);
        if (this->renderFrameShader->getUnf("u_depth")) {
            glUniform1i(this->renderFrameShader->getUnf("u_depth"), 1);
        }
}
void WoWScene::activateTextureCompositionShader(GLuint texture) {
        this.currentShaderProgram = this.textureCompositionShader;
        if (this.currentShaderProgram) {
            var gl = this.gl;
            gl.useProgram(this.currentShaderProgram.program);
            var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

            gl.bindFramebuffer(gl.FRAMEBUFFER, this.textureCompVars.framebuffer);
            gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
            if (this.textureCompVars.depthTexture) {
                gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, this.textureCompVars.depthTexture, 0);
            }

            gl.bindBuffer(gl.ARRAY_BUFFER, this.textureCompVars.textureCoords);
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.textureCompVars.elements);


            gl.vertexAttribPointer(this.currentShaderProgram.shaderAttributes.aTextCoord, 2, gl.FLOAT, false, 0, 0);  // position

            gl.activeTexture(gl.TEXTURE0);
            gl.uniform1i(this.currentShaderProgram.shaderUniforms.uTexture, 0);

            gl.depthMask(true);
            gl.disable(gl.CULL_FACE);

            gl.clearColor(0,0,1,1);
            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
            gl.disable(gl.DEPTH_TEST);
            gl.depthMask(false);
            gl.viewport(0,0,1024,1024)
        }
}
void WoWScene::activateRenderDepthShader () {
    this.currentShaderProgram = this.drawDepthBuffer;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        gl.useProgram(this.currentShaderProgram.program);
        var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();



        gl.activeTexture(gl.TEXTURE0);
    }
}
void WoWScene::activateReadDepthBuffer () {
    this.currentShaderProgram = this.readDepthBuffer;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        gl.useProgram(this.currentShaderProgram.program);
        var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

        gl.activeTexture(gl.TEXTURE0);

        gl.enableVertexAttribArray(this.currentShaderProgram.shaderAttributes.position);
        gl.enableVertexAttribArray(this.currentShaderProgram.shaderAttributes.texture);

    }
}
void WoWScene::activateAdtShader (){
    this.currentShaderProgram = this.adtShader;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        var instExt = this.sceneApi.extensions.getInstancingExt();
        var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

        gl.useProgram(this.currentShaderProgram.program);

        gl.enableVertexAttribArray(shaderAttributes.aHeight);
        gl.enableVertexAttribArray(shaderAttributes.aIndex);

        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uLookAtMat, false, this.lookAtMat4);
        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uPMatrix, false, this.perspectiveMatrix);

        if (this.currentWdt && ((this.currentWdt.flags & 0x04) > 0)) {
            gl.uniform1i(this.currentShaderProgram.shaderUniforms.uNewFormula, 1);
        } else {
            gl.uniform1i(this.currentShaderProgram.shaderUniforms.uNewFormula, 0);
        }

        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uLayer0, 0);
        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uAlphaTexture, 1);
        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uLayer1, 2);
        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uLayer2, 3);
        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uLayer3, 4);

        gl.uniform1f(this.currentShaderProgram.shaderUniforms.uFogStart, this.uFogStart);
        gl.uniform1f(this.currentShaderProgram.shaderUniforms.uFogEnd, this.uFogEnd);

        gl.uniform3fv(this.currentShaderProgram.shaderUniforms.uFogColor, this.fogColor);
    }
}
void WoWScene::activateWMOShader () {
    this.currentShaderProgram = this.wmoShader;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        gl.useProgram(this.currentShaderProgram.program);
        var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

        gl.enableVertexAttribArray(shaderAttributes.aPosition);
        if (shaderAttributes.aNormal) {
            gl.enableVertexAttribArray(shaderAttributes.aNormal);
        }
        gl.enableVertexAttribArray(shaderAttributes.aTexCoord);
        gl.enableVertexAttribArray(shaderAttributes.aTexCoord2);
        gl.enableVertexAttribArray(shaderAttributes.aColor);
        gl.enableVertexAttribArray(shaderAttributes.aColor2);

        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uLookAtMat, false, this.lookAtMat4);
        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uPMatrix, false, this.perspectiveMatrix);

        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uTexture, 0);
        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uTexture2, 1);

        gl.uniform1f(this.currentShaderProgram.shaderUniforms.uFogStart, this.uFogStart);
        gl.uniform1f(this.currentShaderProgram.shaderUniforms.uFogEnd, this.uFogEnd);

        gl.uniform3fv(this.currentShaderProgram.shaderUniforms.uFogColor, this.fogColor);

        gl.activeTexture(gl.TEXTURE0);
    }
}
void WoWScene::deactivateWMOShader () {
    var gl = this.gl;
    var instExt = this.sceneApi.extensions.getInstancingExt();
    var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

    //gl.disableVertexAttribArray(shaderAttributes.aPosition);
    if (shaderAttributes.aNormal) {
        gl.disableVertexAttribArray(shaderAttributes.aNormal);
    }

    gl.disableVertexAttribArray(shaderAttributes.aTexCoord);
    gl.disableVertexAttribArray(shaderAttributes.aTexCoord2);

    gl.disableVertexAttribArray(shaderAttributes.aColor);
    gl.disableVertexAttribArray(shaderAttributes.aColor2);
}
void WoWScene::deactivateTextureCompositionShader() {
    var gl = this.gl;
    gl.useProgram(this.currentShaderProgram.program);
    var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.enable(gl.DEPTH_TEST);
    gl.depthMask(true);
    gl.disable(gl.BLEND);
}

void WoWScene::activateM2ShaderAttribs() {
    var gl = this.gl;
    var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();
    gl.enableVertexAttribArray(shaderAttributes.aPosition);
    if (shaderAttributes.aNormal) {
        gl.enableVertexAttribArray(shaderAttributes.aNormal);
    }
    gl.enableVertexAttribArray(shaderAttributes.boneWeights);
    gl.enableVertexAttribArray(shaderAttributes.bones);
    gl.enableVertexAttribArray(shaderAttributes.aTexCoord);
    gl.enableVertexAttribArray(shaderAttributes.aTexCoord2);
}
void WoWScene::deactivateM2ShaderAttribs() {
    var gl = this.gl;
    var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

    gl.disableVertexAttribArray(shaderAttributes.aPosition);

    if (shaderAttributes.aNormal) {
        gl.disableVertexAttribArray(shaderAttributes.aNormal);
    }
    gl.disableVertexAttribArray(shaderAttributes.boneWeights);
    gl.disableVertexAttribArray(shaderAttributes.bones);

    gl.disableVertexAttribArray(shaderAttributes.aTexCoord);
    gl.disableVertexAttribArray(shaderAttributes.aTexCoord2);
    gl.enableVertexAttribArray(0);
}
void WoWScene::activateM2Shader () {
    this.currentShaderProgram = this.m2Shader;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        gl.useProgram(this.currentShaderProgram.program);
        gl.enableVertexAttribArray(0);
        if (!this.vao_ext) {
            this.activateM2ShaderAttribs()
        }

        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uLookAtMat, false, this.lookAtMat4);
        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uPMatrix, false, this.perspectiveMatrix);
        if (this.currentShaderProgram.shaderUniforms.isBillboard) {
            gl.uniform1i(this.currentShaderProgram.shaderUniforms.isBillboard, 0);
        }

        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uTexture, 0);
        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uTexture2, 1);

        gl.uniform1f(this.currentShaderProgram.shaderUniforms.uFogStart, this.uFogStart);
        gl.uniform1f(this.currentShaderProgram.shaderUniforms.uFogEnd, this.uFogEnd);

        gl.uniform3fv(this.currentShaderProgram.shaderUniforms.uFogColor, this.fogColor);


        gl.activeTexture(gl.TEXTURE0);
    }
}
void WoWScene::deactivateM2Shader () {
    var gl = this.gl;
    var instExt = this.sceneApi.extensions.getInstancingExt();

    if (!this.vao_ext) {
        this.deactivateM2ShaderAttribs()
    }
}
void WoWScene::activateM2InstancingShader () {
    this.currentShaderProgram = this.m2InstancingShader;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        var instExt = this.sceneApi.extensions.getInstancingExt();
        var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

        gl.useProgram(this.currentShaderProgram.program);

        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uLookAtMat, false, this.lookAtMat4);
        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uPMatrix, false, this.perspectiveMatrix);

        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uTexture, 0);
        gl.uniform1i(this.currentShaderProgram.shaderUniforms.uTexture2, 1);

        gl.uniform1f(this.currentShaderProgram.shaderUniforms.uFogStart, this.uFogStart);
        gl.uniform1f(this.currentShaderProgram.shaderUniforms.uFogEnd, this.uFogEnd);

        gl.activeTexture(gl.TEXTURE0);
        gl.enableVertexAttribArray(0);
        gl.enableVertexAttribArray(shaderAttributes.aPosition);
        if (shaderAttributes.aNormal) {
            gl.enableVertexAttribArray(shaderAttributes.aNormal);
        }
        gl.enableVertexAttribArray(shaderAttributes.boneWeights);
        gl.enableVertexAttribArray(shaderAttributes.bones);
        gl.enableVertexAttribArray(shaderAttributes.aTexCoord);
        gl.enableVertexAttribArray(shaderAttributes.aTexCoord2);

        gl.enableVertexAttribArray(shaderAttributes.aPlacementMat + 0);
        gl.enableVertexAttribArray(shaderAttributes.aPlacementMat + 1);
        gl.enableVertexAttribArray(shaderAttributes.aPlacementMat + 2);
        gl.enableVertexAttribArray(shaderAttributes.aPlacementMat + 3);
        gl.enableVertexAttribArray(shaderAttributes.aDiffuseColor);
        if (instExt != null) {
            instExt.vertexAttribDivisorANGLE(shaderAttributes.aPlacementMat + 0, 1);
            instExt.vertexAttribDivisorANGLE(shaderAttributes.aPlacementMat + 1, 1);
            instExt.vertexAttribDivisorANGLE(shaderAttributes.aPlacementMat + 2, 1);
            instExt.vertexAttribDivisorANGLE(shaderAttributes.aPlacementMat + 3, 1);
            instExt.vertexAttribDivisorANGLE(shaderAttributes.aDiffuseColor, 1);
        }

        gl.uniform3fv(this.currentShaderProgram.shaderUniforms.uFogColor, this.fogColor);
    }

}
void WoWScene::deactivateM2InstancingShader () {
    var gl = this.gl;
    var instExt = this.sceneApi.extensions.getInstancingExt();
    var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

    gl.disableVertexAttribArray(shaderAttributes.aPosition);
    if (shaderAttributes.aNormal) {
        gl.disableVertexAttribArray(shaderAttributes.aNormal);
    }
    gl.disableVertexAttribArray(shaderAttributes.boneWeights);
    gl.disableVertexAttribArray(shaderAttributes.bones);
    gl.disableVertexAttribArray(shaderAttributes.aTexCoord);
    gl.disableVertexAttribArray(shaderAttributes.aTexCoord2);

    if (instExt) {
        instExt.vertexAttribDivisorANGLE(shaderAttributes.aPlacementMat + 0, 0);
        instExt.vertexAttribDivisorANGLE(shaderAttributes.aPlacementMat + 1, 0);
        instExt.vertexAttribDivisorANGLE(shaderAttributes.aPlacementMat + 2, 0);
        instExt.vertexAttribDivisorANGLE(shaderAttributes.aPlacementMat + 3, 0);
    }
    gl.disableVertexAttribArray(shaderAttributes.aPlacementMat + 0);
    gl.disableVertexAttribArray(shaderAttributes.aPlacementMat + 1);
    gl.disableVertexAttribArray(shaderAttributes.aPlacementMat + 2);
    gl.disableVertexAttribArray(shaderAttributes.aPlacementMat + 3);

    gl.enableVertexAttribArray(0);
}
void WoWScene::activateBoundingBoxShader () {
    this.currentShaderProgram = this.bbShader;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        gl.useProgram(this.currentShaderProgram.program);

        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.bbBoxVars.ibo_elements);
        gl.bindBuffer(gl.ARRAY_BUFFER, this.bbBoxVars.vbo_vertices);

        //gl.enableVertexAttribArray(this.currentShaderProgram.shaderAttributes.aPosition);
        gl.vertexAttribPointer(this.currentShaderProgram.shaderAttributes.aPosition, 3, gl.FLOAT, false, 0, 0);  // position

        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uLookAtMat, false, this.lookAtMat4);
        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uPMatrix, false, this.perspectiveMatrix);
    }
}
void WoWScene::activateFrustumBoxShader () {
    this.currentShaderProgram = this.drawFrustumShader;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        gl.useProgram(this.currentShaderProgram.program);

        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.bbBoxVars.ibo_elements);
        gl.bindBuffer(gl.ARRAY_BUFFER, this.bbBoxVars.vbo_vertices);

        gl.enableVertexAttribArray(this.currentShaderProgram.shaderAttributes.aPosition);
        gl.vertexAttribPointer(this.currentShaderProgram.shaderAttributes.aPosition, 3, gl.FLOAT, false, 0, 0);  // position

        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uLookAtMat, false, this.lookAtMat4);
        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uPMatrix, false, this.perspectiveMatrix);
    }
}
void WoWScene::activateDrawLinesShader () {
    this.currentShaderProgram = this.drawLinesShader;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        gl.useProgram(this.currentShaderProgram.program);

        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uLookAtMat, false, this.lookAtMat4);
        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uPMatrix, false, this.perspectiveMatrix);
    }
}
void WoWScene::activateDrawPortalShader () {
    this.currentShaderProgram = this.drawPortalShader;
    if (this.currentShaderProgram) {
        var gl = this.gl;
        gl.useProgram(this.currentShaderProgram.program);

        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uLookAtMat, false, this.lookAtMat4);
        gl.uniformMatrix4fv(this.currentShaderProgram.shaderUniforms.uPMatrix, false, this.perspectiveMatrix);
    }
} */
/****************/



void glClearScreen() {
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glDisable(GL_BLEND);
    //gl.clearColor(0.6, 0.95, 1.0, 1);
    //gl.clearColor(0.117647, 0.207843, 0.392157, 1);
    //gl.clearColor(fogColor[0], fogColor[1], fogColor[2], 1);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
}
void WoWScene::draw(int deltaTime) {
    glClearScreen();
}


