#include "ShaderTest2.h"
#include "ShaderTest.h"
#include "../testResource.h"
#include "cocos2d.h"

namespace ShaderTest2
{
    static std::function<Layer*()> createFunctions[] =
    {
        CL(NormalSpriteTest),
        CL(GreyScaleSpriteTest),
        CL(BlurSpriteTest),
        CL(NoiseSpriteTest),
        CL(EdgeDetectionSpriteTest),
        CL(BloomSpriteTest),
        CL(CelShadingSpriteTest),
        CL(LensFlareSpriteTest)
    };
    
    static unsigned int TEST_CASE_COUNT = sizeof(ShaderTest2::createFunctions) / sizeof(ShaderTest2::createFunctions[0]);
    
    static int sceneIdx=-1;
    Layer* createTest(int index)
    {
        auto layer = (createFunctions[index])();;
        
        if (layer)
        {
            layer->autorelease();
        }
        
        return layer;
    }
    
    Layer* nextAction();
    Layer* backAction();
    Layer* restartAction();
    
    Layer* nextAction()
    {
        sceneIdx++;
        sceneIdx = sceneIdx % TEST_CASE_COUNT;
        
        return createTest(sceneIdx);
    }
    
    Layer* backAction()
    {
        sceneIdx--;
        if( sceneIdx < 0 )
            sceneIdx = TEST_CASE_COUNT -1;
        
        return createTest(sceneIdx);
    }
    
    Layer* restartAction()
    {
        return createTest(sceneIdx);
    }
    
}

ShaderTestDemo2::ShaderTestDemo2()
{
    
}

void ShaderTestDemo2::backCallback(Object* sender)
{
    auto s = new ShaderTestScene2();
    s->addChild( ShaderTest2::backAction() );
    Director::getInstance()->replaceScene(s);
    s->release();
}

void ShaderTestDemo2::nextCallback(Object* sender)
{
    auto s = new ShaderTestScene2();//CCScene::create();
    s->addChild( ShaderTest2::nextAction() );
    Director::getInstance()->replaceScene(s);
    s->release();
}

void ShaderTestDemo2::restartCallback(Object* sender)
{
    auto s = new ShaderTestScene2();
    s->addChild(ShaderTest2::restartAction());
    
    Director::getInstance()->replaceScene(s);
    s->release();
}

void ShaderTestScene2::runThisTest()
{
    auto layer = ShaderTest2::nextAction();
    addChild(layer);
    Director::getInstance()->replaceScene(this);
}

template <class spriteType>
class ShaderSpriteCreator
{
public:
    static spriteType* createSprite(const char* pszFileName)
    {
        spriteType* pRet = new spriteType();
        if (pRet && pRet->initWithFile(pszFileName))
        {
            pRet->autorelease();
        }
        else
        {
            CC_SAFE_DELETE(pRet);
        }
        return pRet;
    }
};

class ShaderSprite : public Sprite
{
public:
    ShaderSprite();
    ~ShaderSprite();

    bool initWithTexture(Texture2D* texture, const Rect&  rect);
    void draw();
    void initProgram();
    void listenBackToForeground(Object *obj);
    
protected:
    virtual void buildCustomUniforms() = 0;
    virtual void setCustomUniforms() = 0;
protected:
    std::string _fragSourceFile;

};

ShaderSprite::ShaderSprite()
{
}

ShaderSprite::~ShaderSprite()
{
    NotificationCenter::getInstance()->removeObserver(this, EVNET_COME_TO_FOREGROUND);
}

void ShaderSprite::listenBackToForeground(Object *obj)
{
    setShaderProgram(NULL);
    initProgram();
}

bool ShaderSprite::initWithTexture(Texture2D* texture, const Rect& rect)
{
    if( Sprite::initWithTexture(texture, rect) )
    {
        NotificationCenter::getInstance()->addObserver(this,
                                                       callfuncO_selector(ShaderSprite::listenBackToForeground),
                                                       EVNET_COME_TO_FOREGROUND,
                                                       NULL);
        
        this->initProgram();
        
        return true;
    }
    
    return false;
}

void ShaderSprite::initProgram()
{
    GLchar * fragSource = (GLchar*) String::createWithContentsOfFile(
                                                                     FileUtils::getInstance()->fullPathForFilename(_fragSourceFile.c_str()).c_str())->getCString();
    auto pProgram = new GLProgram();
    pProgram->initWithVertexShaderByteArray(ccPositionTextureColor_vert, fragSource);
    setShaderProgram(pProgram);
    pProgram->release();
    
    CHECK_GL_ERROR_DEBUG();
    
    getShaderProgram()->addAttribute(GLProgram::ATTRIBUTE_NAME_POSITION, GLProgram::VERTEX_ATTRIB_POSITION);
    getShaderProgram()->addAttribute(GLProgram::ATTRIBUTE_NAME_COLOR, GLProgram::VERTEX_ATTRIB_COLOR);
    getShaderProgram()->addAttribute(GLProgram::ATTRIBUTE_NAME_TEX_COORD, GLProgram::VERTEX_ATTRIB_TEX_COORDS);
    
    CHECK_GL_ERROR_DEBUG();
    
    getShaderProgram()->link();
    
    CHECK_GL_ERROR_DEBUG();
    
    getShaderProgram()->updateUniforms();
    
    CHECK_GL_ERROR_DEBUG();
    
    buildCustomUniforms();
    
    CHECK_GL_ERROR_DEBUG();
}

void ShaderSprite::draw()
{
    GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX );
    BlendFunc blend = getBlendFunc();
    GL::blendFunc(blend.src, blend.dst);
    
    getShaderProgram()->use();
    getShaderProgram()->setUniformsForBuiltins();
    setCustomUniforms();
    
    GL::bindTexture2D( getTexture()->getName());
    
    //
    // Attributes
    //
#define kQuadSize sizeof(_quad.bl)
    long offset = (long)&_quad;
    
    // vertex
    int diff = offsetof( V3F_C4B_T2F, vertices);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (void*) (offset + diff));
    
    // texCoods
    diff = offsetof( V3F_C4B_T2F, texCoords);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));
    
    // color
    diff = offsetof( V3F_C4B_T2F, colors);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (void*)(offset + diff));
    
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    CC_INCREMENT_GL_DRAWS(1);
}

class NormalSprite : public ShaderSprite, public ShaderSpriteCreator<NormalSprite>
{
public:
    NormalSprite();
protected:
    virtual void buildCustomUniforms();
    virtual void setCustomUniforms();
};

NormalSprite::NormalSprite()
{
    _fragSourceFile = "Shaders/example_normal.fsh";
}

void NormalSprite::buildCustomUniforms()
{
    
}

void NormalSprite::setCustomUniforms()
{
    
}

class GreyScaleSprite : public ShaderSprite, public ShaderSpriteCreator<GreyScaleSprite>
{
public:
    GreyScaleSprite();
protected:
    virtual void buildCustomUniforms();
    virtual void setCustomUniforms();
};

GreyScaleSprite::GreyScaleSprite()
{
    _fragSourceFile = "Shaders/example_greyScale.fsh";
}

void GreyScaleSprite::buildCustomUniforms()
{
    
}

void GreyScaleSprite::setCustomUniforms()
{
    
}

class BlurSprite : public ShaderSprite, public ShaderSpriteCreator<BlurSprite>
{
public:
    BlurSprite();
    void setBlurSize(float f);
protected:
    virtual void buildCustomUniforms();
    virtual void setCustomUniforms();
protected:
    Point blur_;
    GLfloat    sub_[4];
    
    GLuint    blurLocation;
    GLuint    subLocation;
};

BlurSprite::BlurSprite()
{
    _fragSourceFile = "Shaders/example_Blur.fsh";
}

void BlurSprite::buildCustomUniforms()
{
    auto s = getTexture()->getContentSizeInPixels();
    
    blur_ = Point(1/s.width, 1/s.height);
    sub_[0] = sub_[1] = sub_[2] = sub_[3] = 0;
    
    subLocation = glGetUniformLocation( getShaderProgram()->getProgram(), "substract");
    blurLocation = glGetUniformLocation( getShaderProgram()->getProgram(), "blurSize");
}

void BlurSprite::setCustomUniforms()
{

    getShaderProgram()->setUniformLocationWith2f(blurLocation, blur_.x, blur_.y);
    getShaderProgram()->setUniformLocationWith4fv(subLocation, sub_, 1);
}

void BlurSprite::setBlurSize(float f)
{
    auto s = getTexture()->getContentSizeInPixels();
    
    blur_ = Point(1/s.width, 1/s.height);
    blur_ = blur_ * f;
}

class NoiseSprite : public ShaderSprite, public ShaderSpriteCreator<NoiseSprite>
{
public:
    NoiseSprite();
    
private:
    GLfloat _resolution[2];
    GLuint _resolutionLoc;
protected:
    virtual void buildCustomUniforms();
    virtual void setCustomUniforms();
};

NoiseSprite::NoiseSprite()
{
    _fragSourceFile = "Shaders/example_Noisy.fsh";
}

void NoiseSprite::buildCustomUniforms()
{
    _resolutionLoc = glGetUniformLocation( getShaderProgram()->getProgram(), "resolution");
}

void NoiseSprite::setCustomUniforms()
{
    _resolution[0] = getTexture()->getContentSizeInPixels().width;
    _resolution[1] = getTexture()->getContentSizeInPixels().height;
    
    getShaderProgram()->setUniformLocationWith2fv(_resolutionLoc, _resolution, 1);
}

class EdgeDetectionSprite : public ShaderSprite, public ShaderSpriteCreator<EdgeDetectionSprite>
{
public:
    EdgeDetectionSprite();
    
private:
    GLfloat _resolution[2];
    GLuint _resolutionLoc;
protected:
    virtual void buildCustomUniforms();
    virtual void setCustomUniforms();
};

EdgeDetectionSprite::EdgeDetectionSprite()
{
    _fragSourceFile = "Shaders/example_edgeDetection.fsh";
}

void EdgeDetectionSprite::buildCustomUniforms()
{
    _resolutionLoc = glGetUniformLocation( getShaderProgram()->getProgram(), "resolution");
}

void EdgeDetectionSprite::setCustomUniforms()
{
    _resolution[0] = getTexture()->getContentSizeInPixels().width;
    _resolution[1] = getTexture()->getContentSizeInPixels().height;
    
    getShaderProgram()->setUniformLocationWith2fv(_resolutionLoc, _resolution, 1);
}

class BloomSprite : public ShaderSprite, public ShaderSpriteCreator<BloomSprite>
{
public:
    BloomSprite();
    
private:
    GLfloat _resolution[2];
    GLuint _resolutionLoc;
protected:
    virtual void buildCustomUniforms();
    virtual void setCustomUniforms();
};

BloomSprite::BloomSprite()
{
    _fragSourceFile = "Shaders/example_bloom.fsh";
}

void BloomSprite::buildCustomUniforms()
{
    _resolutionLoc = glGetUniformLocation( getShaderProgram()->getProgram(), "resolution");
}

void BloomSprite::setCustomUniforms()
{
    _resolution[0] = getTexture()->getContentSizeInPixels().width;
    _resolution[1] = getTexture()->getContentSizeInPixels().height;
    
    getShaderProgram()->setUniformLocationWith2fv(_resolutionLoc, _resolution, 1);
}

class CelShadingSprite : public ShaderSprite, public ShaderSpriteCreator<CelShadingSprite>
{
public:
    CelShadingSprite();
    
private:
    GLfloat _resolution[2];
    GLuint _resolutionLoc;
protected:
    virtual void buildCustomUniforms();
    virtual void setCustomUniforms();
};

CelShadingSprite::CelShadingSprite()
{
    _fragSourceFile = "Shaders/example_celShading.fsh";
}

void CelShadingSprite::buildCustomUniforms()
{
    _resolutionLoc = glGetUniformLocation( getShaderProgram()->getProgram(), "resolution");
}

void CelShadingSprite::setCustomUniforms()
{
    _resolution[0] = getTexture()->getContentSizeInPixels().width;
    _resolution[1] = getTexture()->getContentSizeInPixels().height;
    
    getShaderProgram()->setUniformLocationWith2fv(_resolutionLoc, _resolution, 1);
}

class LensFlareSprite : public ShaderSprite, public ShaderSpriteCreator<LensFlareSprite>
{
public:
    LensFlareSprite();
    
private:
    GLfloat _resolution[2];
    GLfloat _textureResolution[2];
    GLuint _resolutionLoc;
    GLuint _textureResolutionLoc;
protected:
    virtual void buildCustomUniforms();
    virtual void setCustomUniforms();
};

LensFlareSprite::LensFlareSprite()
{
    _fragSourceFile = "Shaders/example_lensFlare.fsh";
}

void LensFlareSprite::buildCustomUniforms()
{
    _resolutionLoc = glGetUniformLocation( getShaderProgram()->getProgram(), "resolution");
    _textureResolutionLoc = glGetUniformLocation( getShaderProgram()->getProgram(), "textureResolution");
}

void LensFlareSprite::setCustomUniforms()
{
    _textureResolution[0] = getTexture()->getContentSizeInPixels().width;
    _textureResolution[1] = getTexture()->getContentSizeInPixels().height;
    
    _resolution[0] = getContentSize().width;
    _resolution[1] = getContentSize().height;
    
    getShaderProgram()->setUniformLocationWith2fv(_resolutionLoc, _resolution, 1);
    getShaderProgram()->setUniformLocationWith2fv(_textureResolutionLoc, _textureResolution, 1);
}

NormalSpriteTest::NormalSpriteTest()
{
    if (ShaderTestDemo2::init())
    {
        auto s = Director::getInstance()->getWinSize();
        NormalSprite* sprite = NormalSprite::createSprite("Images/powered.png");
        sprite->setPosition(Point(s.width/2, s.height/2));
        addChild(sprite);
    }
    
}

GreyScaleSpriteTest::GreyScaleSpriteTest()
{
    if (ShaderTestDemo2::init())
    {
        auto s = Director::getInstance()->getWinSize();
        GreyScaleSprite* sprite = GreyScaleSprite::createSprite("Images/powered.png");
        sprite->setPosition(Point(s.width * 0.75, s.height/2));
        auto sprite2 = Sprite::create("Images/powered.png");
        sprite2->setPosition(Point(s.width * 0.25, s.height/2));
        addChild(sprite);
        addChild(sprite2);
    }
    
}

BlurSpriteTest::BlurSpriteTest()
{
    if (ShaderTestDemo2::init())
    {
        auto s = Director::getInstance()->getWinSize();
        BlurSprite* sprite = BlurSprite::createSprite("Images/powered.png");
        sprite->setPosition(Point(s.width * 0.75, s.height/2));
        auto sprite2 = Sprite::create("Images/powered.png");
        sprite2->setPosition(Point(s.width * 0.25, s.height/2));
        addChild(sprite);
        addChild(sprite2);
    }
    
}

NoiseSpriteTest::NoiseSpriteTest()
{
    if (ShaderTestDemo2::init())
    {
        auto s = Director::getInstance()->getWinSize();
        NoiseSprite* sprite = NoiseSprite::createSprite("Images/powered.png");
        sprite->setPosition(Point(s.width * 0.75, s.height/2));
        auto sprite2 = Sprite::create("Images/powered.png");
        sprite2->setPosition(Point(s.width * 0.25, s.height/2));
        addChild(sprite);
        addChild(sprite2);
    }
}

EdgeDetectionSpriteTest::EdgeDetectionSpriteTest()
{
    if (ShaderTestDemo2::init())
    {
        auto s = Director::getInstance()->getWinSize();
        EdgeDetectionSprite* sprite = EdgeDetectionSprite::createSprite("Images/powered.png");
        sprite->setPosition(Point(s.width * 0.75, s.height/2));
        auto sprite2 = Sprite::create("Images/powered.png");
        sprite2->setPosition(Point(s.width * 0.25, s.height/2));
        addChild(sprite);
        addChild(sprite2);
    }
}

BloomSpriteTest::BloomSpriteTest()
{
    if (ShaderTestDemo2::init())
    {
        auto s = Director::getInstance()->getWinSize();
        BloomSprite* sprite = BloomSprite::createSprite("Images/stone.png");
        sprite->setPosition(Point(s.width * 0.75, s.height/2));
        auto sprite2 = Sprite::create("Images/stone.png");
        sprite2->setPosition(Point(s.width * 0.25, s.height/2));
        addChild(sprite);
        addChild(sprite2);
    }
}

CelShadingSpriteTest::CelShadingSpriteTest()
{
    if (ShaderTestDemo2::init())
    {
        auto s = Director::getInstance()->getWinSize();
        CelShadingSprite* sprite = CelShadingSprite::createSprite("Images/stone.png");
        sprite->setPosition(Point(s.width * 0.75, s.height/2));
        auto sprite2 = Sprite::create("Images/stone.png");
        sprite2->setPosition(Point(s.width * 0.25, s.height/2));
        addChild(sprite);
        addChild(sprite2);
    }
}

LensFlareSpriteTest::LensFlareSpriteTest()
{
    if (ShaderTestDemo2::init())
    {
        auto s = Director::getInstance()->getWinSize();
        LensFlareSprite* sprite = LensFlareSprite::createSprite("Images/noise.png");
        Rect rect = Rect::ZERO;
        rect.size = Size(480,320);
        sprite->setPosition(Point(s.width * 0.5, s.height/2));
        addChild(sprite);
    }
}
