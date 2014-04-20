#include "GameLayer.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;

GameLayer::~GameLayer () {
    
    //release all retained actions
    CC_SAFE_RELEASE(_growBomb);
    CC_SAFE_RELEASE(_rotateSprite);
    CC_SAFE_RELEASE(_shockwaveSequence);
    CC_SAFE_RELEASE(_swingHealth);
    CC_SAFE_RELEASE(_groundHit);
    CC_SAFE_RELEASE(_explosion);
    
    //release all retained arrays
    CC_SAFE_RELEASE(_clouds);
    CC_SAFE_RELEASE(_meteorPool);
    CC_SAFE_RELEASE(_healthPool);
    CC_SAFE_RELEASE(_fallingObjects);
	
}

CCScene* GameLayer::scene()
{
    CCScene* scene = CCScene::create();
    GameLayer *gameLayer = GameLayer::create();
    scene->addChild(gameLayer);
    return scene;
}

GameLayer* GameLayer::create()
{
    GameLayer *gameLayer = new GameLayer();
    if(gameLayer && gameLayer->init())
    {
        gameLayer->autorelease();
        return gameLayer;
    }
    else
    {
        delete gameLayer;
        gameLayer = NULL;
        return gameLayer;
    }
}

bool GameLayer::init()
{
    //Call Super init
    if(!CCLayer::init()){
        return false;
    }
    
    //set screen size
    _screenSize = CCDirector::sharedDirector()->getWinSize();
    
    //set running = false
    _running = false;
    
    createGameScreen();
    createObjectPools();
    createActions();
    
    _fallingObjects = CCArray::createWithCapacity(40); //_fallingObjects will be added to autorelease pool, so retain it.
    _fallingObjects->retain();
    
    this->setTouchEnabled(true);
    
    this->schedule(schedule_selector(GameLayer::update));
        
    if((rand() % 10) % 2 == 0){
        SimpleAudioEngine::sharedEngine()->playBackgroundMusic("nocturne.mp3", true);
    }
    else{
        SimpleAudioEngine::sharedEngine()->playBackgroundMusic("singularity.mp3", true);
    }
    
    return true;
}

void GameLayer::createGameScreen()
{
    CCSprite *bgSprite = CCSprite::create("bg.png");
    bgSprite->setAnchorPoint(ccp(0,0));
    bgSprite->setPosition(ccp(0, 0));
    this->addChild(bgSprite);
    
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("sprite_sheet.plist");
    
    _gameBatchNode = CCSpriteBatchNode::create("sprite_sheet.png");
    this->addChild(_gameBatchNode);
    
    CCSprite* sprite;
    
    for(int i = 0; i < 2; i++)
    {
        sprite = CCSprite::createWithSpriteFrameName("city_dark.png");
        sprite->setPosition(ccp(_screenSize.width * (0.25f + (i * 0.5f)),sprite->boundingBox().size.height * 0.5f));
        _gameBatchNode->addChild(sprite);
        
        sprite = CCSprite::createWithSpriteFrameName("city_light.png");
        sprite->setPosition(ccp(_screenSize.width * (0.25f + (i * 0.5f)),sprite->boundingBox().size.height * 0.9f));
        _gameBatchNode->addChild(sprite);
    }
    
    for (int i = 0; i < 3; i++)
    {
        sprite = CCSprite::createWithSpriteFrameName("trees.png");
        sprite->setPosition(ccp(_screenSize.width * (0.2f + i * 0.3f),sprite->boundingBox().size.height * 0.5f));
        _gameBatchNode->addChild(sprite, kForeground);
    }
    
    _scoreDisplay = CCLabelBMFont::create("0","font.fnt", _screenSize.width * 0.3f);
    _scoreDisplay->setAnchorPoint(ccp(1,0.5));
    _scoreDisplay->setPosition(ccp(_screenSize.width * 0.8f, _screenSize.height * 0.94f));
    this->addChild(_scoreDisplay);
    
    _energyDisplay = CCLabelBMFont::create("100%","font.fnt",_screenSize.width * 0.1f, kCCTextAlignmentRight);
    _energyDisplay->setPosition(ccp(_screenSize.width * 0.3f, _screenSize.height * 0.94f));
    this->addChild(_energyDisplay);
    
    CCSprite *icon = CCSprite::createWithSpriteFrameName("health_icon.png");
    icon->setPosition(ccp(_screenSize.width * 0.15f, _screenSize.height * 0.94));
    _gameBatchNode->addChild(icon, kBackground);
    
    CCSprite *cloud;
    _clouds = CCArray::createWithCapacity(4);
    _clouds->retain();
    
    float cloud_y;
    float cloud_x;
    CCLog("Screen Size: %fx, %fy", _screenSize.width, _screenSize.height);
    for (int i = 0 ; i < 4; i++)
    {
        cloud_y = i % 2 == 0 ? _screenSize.height * 0.4f : _screenSize.height * 0.5f;
        cloud_x = _screenSize.width * 0.1 + (i * (_screenSize.width * 0.3f));
        cloud = CCSprite::createWithSpriteFrameName("cloud.png");
        cloud->setPosition(ccp(cloud_x, cloud_y));
        _gameBatchNode->addChild(cloud,kBackground);
        _clouds->addObject(cloud);
    }
    
    _bomb = CCSprite::createWithSpriteFrameName("bomb.png");
    _bomb->getTexture()->generateMipmap();
    _bomb->setVisible(false);
    
    CCSize size = _bomb->boundingBox().size;
    
    CCSprite *sparkle = CCSprite::createWithSpriteFrameName("sparkle.png");
    sparkle->setPosition(ccp(size.width * 0.72f, size.height * 0.72f));
    _bomb->addChild(sparkle, kMiddleground, kSpriteSparkle);
    
    CCSprite *halo = CCSprite::createWithSpriteFrameName("halo.png");
    halo->setPosition(ccp(size.width * 0.4f, size.height * 0.4f));
    _bomb->addChild(halo, kMiddleground, kSpriteHalo);
    
    _gameBatchNode->addChild(_bomb, kForeground);
    
    _shockWave = CCSprite::createWithSpriteFrameName("shockwave.png");
    _shockWave->getTexture()->generateMipmap();
    _shockWave->setVisible(false);
    
    _gameBatchNode->addChild(_shockWave);
    
    _introMessage = CCSprite::createWithSpriteFrameName("logo.png");
    _introMessage->setPosition(ccp(_screenSize.width * 0.5f, _screenSize.height * 0.6f));
    _introMessage->setVisible(true);
    
    _gameBatchNode->addChild(_introMessage);
    
    _gameOverMessage = CCSprite::createWithSpriteFrameName("gameover.png");
    _gameOverMessage->setPosition(ccp(_screenSize.width * 0.5, _screenSize.height * 0.65f));
    _gameOverMessage->setVisible(false);
    
    _gameBatchNode->addChild(_gameOverMessage);
    
}

void GameLayer::createObjectPools()
{
    int i = 0;
    CCSprite *sprite;
    
    //Create METEOR pool
    _meteorPool = CCArray::createWithCapacity(50);
    _meteorPool->retain();
    _meteorPoolIndex = 0;
    
    for (i = 0; i < 50 ; i++)
    {
        sprite = CCSprite::createWithSpriteFrameName("meteor.png");
        sprite->setVisible(false);
        
        _gameBatchNode->addChild(sprite, kMiddleground, kSpriteMeteor);
        _meteorPool->addObject(sprite);
    }
    
    //Create HEALTH pool
    _healthPool = CCArray::createWithCapacity(20);
    _healthPool->retain();
    _healthPoolIndex = 0;
    
    for (i = 0; i < 20 ; i++)
    {
        sprite = CCSprite::createWithSpriteFrameName("health.png");
        sprite->setVisible(false);
        
        _gameBatchNode->addChild(sprite, kMiddleground, kSpriteHealth);
        _healthPool->addObject(sprite);
    }
}

void GameLayer::createActions()
{
    CCFiniteTimeAction *easeSwing = CCSequence::create(
                                                       CCEaseInOut::create(CCRotateTo::create(1.2f, -10), 2),
                                                       CCEaseInOut::create(CCRotateTo::create(1.2f,  10), 2),
                                                       NULL
                                                       );
    
    _swingHealth = CCRepeatForever::create((CCActionInterval *) easeSwing);
    _swingHealth->retain();
    
    _shockwaveSequence = CCSequence::create(
                                            CCFadeOut::create(1.0f),
                                            CCCallFunc::create(this, callfunc_selector(GameLayer::shockwaveDone)),
                                            NULL
                                            );
    _shockwaveSequence->retain();
    
    _growBomb = CCScaleTo::create(6.0f, 1.0);
    _growBomb->retain();
    
    CCActionInterval *rotate = CCRotateBy::create(0.5f, -90);
    _rotateSprite = CCRepeatForever::create(rotate);
    _rotateSprite->retain();
    
    CCAnimation *animation;
    CCSpriteFrame *frame;
    
    animation = CCAnimation::create();
    for (int i = 0; i < 10; i++)
    {
        CCString *name = CCString::createWithFormat("boom%i.png",i+1);
        frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(name->getCString());
        animation->addSpriteFrame(frame);
    }
    
    //Therefore it is setDelayPerUnit(totalNumberOfSeconds / NumberOfFrames)
    animation->setDelayPerUnit(1/10.0f); //Delay between each frame of the animation. So, the animation takes an entire second for 10 frames.
    animation->setRestoreOriginalFrame(true);
    
    _groundHit = CCSequence::create(
                                    CCMoveBy::create(0, ccp(0, _screenSize.height * 0.12f)), //Move to that point immediately
                                    CCAnimate::create(animation),
                                    CCCallFuncN::create(this, callfuncN_selector(GameLayer::animationDone)),
                                    NULL
                                    );
    _groundHit->retain();
    
    animation = CCAnimation::create();
    for (int i = 0; i < 7; i++)
    {
        CCString *name = CCString::createWithFormat("explosion_small%i.png",i+1);
        //TODO:: Why are we not using CCSprite::createWithSpriteFrameName();
        frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(name->getCString());
        animation->addSpriteFrame(frame);
    }
    
    animation->setDelayPerUnit(0.5/7.0f); //Delay between each frame of the animation. // Animation takes half a second for 7 frames
    animation->setRestoreOriginalFrame(true);
    
    _explosion = CCSequence::create(
                                    CCAnimate::create(animation),
                                    CCCallFuncN::create(this, callfuncN_selector(GameLayer::animationDone)),
                                    NULL
                                    );
    _explosion->retain();
}

void GameLayer::shockwaveDone()
{
    _shockWave->setVisible(false);
}

void GameLayer::animationDone(CCNode *pSender)
{
    pSender->setVisible(false);
}

void GameLayer::resetGame()
{
    _score = 0;
    _energy = 100;
    
    _meteorInterval = 2.5;
    _meteorTimer = _meteorInterval * 0.99f;
    _meteorSpeed = 10;//seconds to reach the ground.
    
    _healthInterval = 20;
    _healthTimer = 0;
    _healthSpeed = 15; //seconds to reach the ground.
    
    _difficultyInterval = 60;
    _difficultyTimer = 0;
    
    _running = true;
    
    //reset labels
    
    CCString *value = CCString::createWithFormat("%i%s", _energy, "%");
    _energyDisplay->setString(value->getCString());
    
    value = CCString::createWithFormat("%i", _score);
    _scoreDisplay->setString(value->getCString());
    SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    if((rand() % 10) % 2 == 0){
        SimpleAudioEngine::sharedEngine()->playBackgroundMusic("racecar.mp3", true);
    }
    else{
        SimpleAudioEngine::sharedEngine()->playBackgroundMusic("cafo.mp3", true);
    }
    
}

void GameLayer::stopGame()
{
    CCSprite *sprite;
    
    _running = false;
    
    int countOfFallingObjects = _fallingObjects->count();
    
    for (int i = countOfFallingObjects - 1; i >= 0; i--) {
        sprite = (CCSprite *) _fallingObjects->objectAtIndex(i);
        sprite->stopAllActions();
        sprite->setVisible(false);
        _fallingObjects->removeObjectAtIndex(i);
    }
    
    if(_bomb->isVisible()){
        CCSprite *child;
        _bomb->stopAllActions();
        _bomb->setVisible(false);
        
        child = (CCSprite *) _bomb->getChildByTag(kSpriteHalo);
        child->stopAllActions();
        child->setVisible(false);
        
        child = (CCSprite *) _bomb->getChildByTag(kSpriteSparkle);
        child->stopAllActions();
        child->setVisible(false);
    }
    
    if(_shockWave->isVisible()){
        _shockWave->stopAllActions();
        _shockWave->setVisible(false);
    }
    _gameOverMessage->setVisible(true); 
    SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
}

void GameLayer::resetMeteors()
{
    if (_fallingObjects->count() > 30) {
        return;
    }
    
    CCSprite *meteorSprite = (CCSprite *) _meteorPool->objectAtIndex(_meteorPoolIndex);
    _meteorPoolIndex++;
    
    if(_meteorPoolIndex >= _meteorPool->count()){
        _meteorPoolIndex = 0;
    }
    
    float meteor_start_x = rand() % (int) _screenSize.width * 0.8f + _screenSize.width * 0.1f;
    float meteor_start_y = _screenSize.height + meteorSprite->boundingBox().size.height * 0.5f;
    
    float meteor_target_x = rand() % (int) _screenSize.width * 0.8f + _screenSize.width * 0.1f;
    float meteor_target_y = _screenSize.height * 0.15f;
    
    CCFiniteTimeAction *sequence = CCSequence::create(
                                                      CCMoveTo::create(_meteorSpeed, ccp(meteor_target_x, meteor_target_y)),
                                                      CCCallFuncN::create(this, callfuncN_selector(GameLayer::fallingObjectsDone)),
                                                      NULL
                                                      );
    
    CCActionInterval *rotate = CCRotateBy::create(0.5f, -90);
    CCFiniteTimeAction *rotateInfinite = CCRepeatForever::create(rotate);
    meteorSprite->setVisible(true);
    meteorSprite->setPosition(ccp(meteor_start_x, meteor_start_y));
    meteorSprite->runAction(rotateInfinite);
    meteorSprite->runAction(sequence);
    _fallingObjects->addObject(meteorSprite);
}

void GameLayer::resetHealthPacks()
{
    if (_fallingObjects->count() > 30) {
        return;
    }
    
    CCSprite *healthSprite = (CCSprite *) _healthPool->objectAtIndex(_healthPoolIndex);
    _healthPoolIndex++;
    
    if(_healthPoolIndex >= _healthPool->count()){
        _healthPoolIndex = 0;
    }
    
    float health_start_x = rand() % (int) _screenSize.width * 0.8f + _screenSize.width * 0.1f;
    float health_start_y = _screenSize.height + healthSprite->boundingBox().size.height * 0.5f;
    
    float health_target_x = rand() % (int) _screenSize.width * 0.8f + _screenSize.width * 0.1f;
    float health_target_y = _screenSize.height * 0.15f;
    
    CCFiniteTimeAction *sequence = CCSequence::create(
                                                      CCMoveTo::create(_healthSpeed, ccp(health_target_x, health_target_y)),
                                                      CCCallFuncN::create(this, callfuncN_selector(GameLayer::fallingObjectsDone)),
                                                      NULL
                                                      );
    
    healthSprite->setVisible(true);
    healthSprite->setPosition(ccp(health_start_x, health_start_y));
    healthSprite->runAction(_swingHealth);
    healthSprite->runAction(sequence);
    _fallingObjects->addObject(healthSprite);
}

void GameLayer::fallingObjectsDone(CCNode *pSender)
{
    _fallingObjects->removeObject(pSender);
    pSender->stopAllActions();
    pSender->setRotation(0);
    
    if (pSender->getTag() == kSpriteMeteor) {
        _energy -= 15;
        pSender->runAction((CCAction *)_groundHit->copy()->autorelease());
        SimpleAudioEngine::sharedEngine()->playEffect("boom.wav");
    }
    else{
        pSender->setVisible(false);
        if(_energy == 100){
            _score += 25;
            CCString *scoreString = CCString::createWithFormat("%i",_score);
            _scoreDisplay->setString(scoreString->getCString());
        }
        else{
            _energy += 10;
            if (_energy > 100) {
                _energy = 100;
            }
        }
        SimpleAudioEngine::sharedEngine()->playEffect("health.wav");
    }
    
    if(_energy <= 0){
        _energy = 0;
        this->stopGame();
        if((rand() % 10) % 2 == 0){
            SimpleAudioEngine::sharedEngine()->playBackgroundMusic("nocturne.mp3", true);
        }
        else{
            SimpleAudioEngine::sharedEngine()->playBackgroundMusic("singularity.mp3", true);
        }
    }
    
    CCString *energyString = CCString::createWithFormat("%i%s",_energy,"%");
    _energyDisplay->setString(energyString->getCString());
}

void GameLayer::ccTouchesBegan(CCSet* pTouches, CCEvent* event)
{
    if (!_running) {
        //if intro, hide intro message
        if (_introMessage->isVisible()) {
            _introMessage->setVisible(false);
            //if game over, hide game over message
        } else if (_gameOverMessage->isVisible()) {
            SimpleAudioEngine::sharedEngine()->stopAllEffects();
            _gameOverMessage->setVisible(false);
        }
        this->resetGame();
        return;
    }
    
    CCTouch *touch = (CCTouch *) pTouches->anyObject();
    if(touch){
        //If bomb is already growing
        if(_bomb->isVisible()){
            //Stop all actions on bomb, halo and sprite
            
            _bomb->stopAllActions();
            
            CCSprite *child;
            child = (CCSprite *) _bomb->getChildByTag(kSpriteHalo);
            child->stopAllActions();
            
            child = (CCSprite *) _bomb->getChildByTag(kSpriteSparkle);
            child->stopAllActions();
            
            if(_bomb->getScale() > 0.3f){
                _shockWave->setScale(0.1f);
                _shockWave->setPosition(_bomb->getPosition());
                _shockWave->setVisible(true);
                _shockWave->runAction(CCScaleTo::create(0.5f, _bomb->getScale() * 2.0f));
                _shockWave->runAction((CCFiniteTimeAction *) _shockwaveSequence->copy()->autorelease());
                SimpleAudioEngine::sharedEngine()->playEffect("bombRelease.wav");
            }
            else{
                SimpleAudioEngine::sharedEngine()->playEffect("bombFail.wav");
            }
            
            _bomb->setVisible(false);
            _shockWaveHits = 0;
        }
        else{
            //create the bomb
            CCPoint tap = touch->getLocation();
            _bomb->stopAllActions();
            _bomb->setScale(0.1f);
            _bomb->setPosition(tap);
            _bomb->setVisible(true);
            _bomb->setOpacity(50);
            _bomb->runAction((CCAction *) _growBomb->copy()->autorelease());
            
            CCSprite *child;
            child = (CCSprite *) _bomb->getChildByTag(kSpriteHalo);
            child->runAction((CCAction *) _rotateSprite->copy()->autorelease());
            
            child = (CCSprite *) _bomb->getChildByTag(kSpriteSparkle);
            child->runAction((CCAction *) _rotateSprite->copy()->autorelease());
        }
    }
}

void GameLayer::update(float dt)
{
    int count;
    CCSprite *sprite;
    
    if(!_running) return;
    
    _meteorTimer += dt;
    if(_meteorTimer > _meteorInterval){
        _meteorTimer = 0;
        this->resetMeteors();
    }
    
    _healthTimer += dt;
    if(_healthTimer > _healthInterval){
        _healthTimer = 0;
        this->resetHealthPacks();
    }
    
    //check collision with shockwave
    if(_shockWave->isVisible()){
        count = _fallingObjects->count();
        for (int i = count - 1; i >= 0; i--) {
            sprite = (CCSprite *)_fallingObjects->objectAtIndex(i);
            float dx = _shockWave->getPositionX() - sprite->getPositionX();
            float dy = _shockWave->getPositionY() - sprite->getPositionY();
            float distance = pow(dx, 2) + pow(dy, 2);
            
            if (distance <= pow(_shockWave->boundingBox().size.width * 0.5f,2)) {
                sprite->stopAllActions();
                sprite->runAction((CCAction *) _explosion->copy()->autorelease());
                SimpleAudioEngine::sharedEngine()->playEffect("boom.wav");
                if (sprite->getTag() == kSpriteMeteor) {
                    _shockWaveHits++;
                    _score += _shockWaveHits * 13 + _shockWaveHits * 2;
                }
                _fallingObjects->removeObjectAtIndex(i);
            }
        }
        CCString *value = CCString::createWithFormat("%i", _score);
        _scoreDisplay->setString(value->getCString());
    }
    
    //moveClouds
    count = _clouds->count();
    for (int i = count - 1; i >= 0; i--) {
        sprite = (CCSprite *) _clouds->objectAtIndex(i);
        sprite->setPositionX(sprite->getPositionX() + dt * 20);
        if(sprite->getPositionX() > _screenSize.width + sprite->boundingBox().size.width * 0.5f){
            sprite->setPositionX(-1 * sprite->boundingBox().size.width * 0.5f);
        }
    }
    
    if (_bomb->isVisible()) {
        if (_bomb->getScale() > 0.3f) {
            if (_bomb->getOpacity() != 255)
                _bomb->setOpacity(255);
        }
    }
}











