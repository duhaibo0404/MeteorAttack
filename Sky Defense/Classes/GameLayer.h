#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#include "SimpleAudioEngine.h"
#include "cocos2d.h"

USING_NS_CC;
using namespace CocosDenshion;

enum {
    kSpriteBomb,
    kSpriteShockwave,
    kSpriteMeteor,
    kSpriteHealth,
    kSpriteHalo,
    kSpriteSparkle
};

enum {
    kBackground,
    kMiddleground,
    kForeground
};


class GameLayer : public cocos2d::CCLayer
{
private:
    ~GameLayer();
    CCArray *_fallingObjects;
    CCArray *_clouds;
    CCArray *_meteorPool;
    CCArray *_healthPool;
    int _meteorPoolIndex;
    int _healthPoolIndex;
    
    int _score;
    int _energy;
    
    float _meteorInterval;
    float _meteorTimer;
    float _meteorSpeed;
    
    float _healthInterval;
    float _healthTimer;
    float _healthSpeed;
    
    float _difficultyInterval;
    float _difficultyTimer;
    float _shockWaveHits;
    
    bool _running;
    
    CCSprite *_bomb;
    CCSprite *_shockWave;
    CCSprite *_introMessage;
    CCSprite *_gameOverMessage;
    
    CCAction *_swingHealth;
    CCAction *_growBomb;
    CCAction *_rotateSprite;
    CCAction *_shockwaveSequence;
    CCAction *_groundHit;
    CCAction *_explosion;
    
    
    CCSpriteBatchNode* _gameBatchNode;
    
    CCLabelBMFont *_scoreDisplay;
    CCLabelBMFont *_energyDisplay;
    
    CCSize _screenSize;
    
    void createGameScreen(void);
    void createObjectPools(void);
    void createActions(void);
    
    void animationDone(CCNode *node);
    void shockwaveDone();
    
    void resetGame();
    void stopGame();
    
    void resetMeteors();
    void resetHealthPacks();
    void fallingObjectsDone(CCNode *node);
    
protected:
    
public:
    virtual bool init(void);
    static cocos2d::CCScene* scene(void);
    static GameLayer* create(void); //CREATE_FUNC(GameLayer);
    virtual void ccTouchesBegan(CCSet* pTouches, CCEvent* event);
    virtual void update (float dt);
};

#endif // __HELLOWORLD_SCENE_H__
