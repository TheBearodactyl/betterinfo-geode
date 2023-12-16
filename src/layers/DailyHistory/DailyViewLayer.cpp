#include "DailyViewLayer.h"
#include "../JumpToPageLayer.h"
#include "../../utils.hpp"

DailyViewLayer* DailyViewLayer::create(bool isWeekly) {
    auto ret = new DailyViewLayer();
    if (ret && ret->init(isWeekly)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool DailyViewLayer::compareDailies(const void* l1, const void* l2){
    const GJGameLevel* level1 = reinterpret_cast<const GJGameLevel*>(l1);
    const GJGameLevel* level2 = reinterpret_cast<const GJGameLevel*>(l2);
    return const_cast<geode::SeedValueRSV&>(level1->m_dailyID).value() < const_cast<geode::SeedValueRSV&>(level2->m_dailyID).value();
}

bool DailyViewLayer::init(bool isWeekly) {
    m_isWeekly = isWeekly;

    auto GLM = GameLevelManager::sharedState();
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto backgroundSprite = CCSprite::create("game_bg_14_001.png");
    bool controllerConnected = BetterInfo::controllerConnected();

    backgroundSprite->setScale(winSize.width / backgroundSprite->getContentSize().width);
    backgroundSprite->setAnchorPoint({0, 0});
    backgroundSprite->setPosition({0,-75});
    backgroundSprite->setColor({164, 0, 255}); //purple
    backgroundSprite->setZOrder(-2);
    addChild(backgroundSprite);

    auto menu = CCMenu::create();
    addChild(menu);
    
    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
        this,
        menu_selector(DailyViewLayer::onBack)
    );

    backBtn->setPosition({(-winSize.width / 2) + 25, (winSize.height / 2) - 25});
    menu->addChild(backBtn);

    setTouchEnabled(true);
    setKeypadEnabled(true);

    auto dailyLevels = GLM->m_dailyLevels;
    m_sortedLevels = CCArray::create();
    m_sortedLevels->retain();
    CCDictElement* obj;
    CCDICT_FOREACH(dailyLevels, obj){
        auto currentLvl = static_cast<GJGameLevel*>(obj->getObject());
        if(
            currentLvl != nullptr &&
            ((isWeekly && currentLvl->m_dailyID >= 100000) || (!isWeekly && currentLvl->m_dailyID < 100000))
        ){
            m_sortedLevels->addObject(currentLvl);
        }
    }
    std::sort(m_sortedLevels->data->arr, m_sortedLevels->data->arr + m_sortedLevels->data->num, DailyViewLayer::compareDailies);

    auto prevSprite = CCSprite::createWithSpriteFrameName(controllerConnected ? "controllerBtn_DPad_Left_001.png" : "GJ_arrow_03_001.png");
    m_prevBtn = CCMenuItemSpriteExtra::create(
        prevSprite,
        this,
        menu_selector(DailyViewLayer::onPrev)
    );
    m_prevBtn->setPosition({- (winSize.width / 2) + 25, 0});
    menu->addChild(m_prevBtn);

    auto nextSprite = CCSprite::createWithSpriteFrameName(controllerConnected ? "controllerBtn_DPad_Right_001.png" : "GJ_arrow_03_001.png");
    if(!controllerConnected) nextSprite->setFlipX(true);
    m_nextBtn = CCMenuItemSpriteExtra::create(
        nextSprite,
        this,
        menu_selector(DailyViewLayer::onNext)
    );
    m_nextBtn->setPosition({+ (winSize.width / 2) - 25, 0});
    menu->addChild(m_nextBtn);

    m_counter = CCLabelBMFont::create("0 to 0 of 0", "goldFont.fnt");
    m_counter->setAnchorPoint({ 1.f, 1.f });
    m_counter->setPosition(winSize - CCPoint(7,3));
    m_counter->setScale(0.5f);
    addChild(m_counter);

    //corners
    auto cornerBL = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    cornerBL->setPosition({0,0});
    cornerBL->setAnchorPoint({0,0});
    addChild(cornerBL, -1);

    auto cornerBR = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    cornerBR->setPosition({winSize.width,0});
    cornerBR->setAnchorPoint({0,0});
    cornerBR->setRotation(270);
    addChild(cornerBR, -1);

    //navigation buttons
    m_pageBtnSprite = ButtonSprite::create("1", 23, true, "bigFont.fnt", "GJ_button_02.png", 40, .7f);
    m_pageBtnSprite->setScale(0.7f);
    auto pageBtn = CCMenuItemSpriteExtra::create(
        m_pageBtnSprite,
        this,
        menu_selector(DailyViewLayer::onJumpToPageLayer)
    );
    pageBtn->setSizeMult(1.2f);
    pageBtn->setPosition({+ (winSize.width / 2) - 23, (winSize.height / 2) - 37});
    menu->addChild(pageBtn);

    auto randomSprite = BetterInfo::createWithBISpriteFrameName("BI_randomBtn_001.png");
    randomSprite->setScale(0.9f);
    auto randomBtn = CCMenuItemSpriteExtra::create(
        randomSprite,
        this,
        menu_selector(DailyViewLayer::onRandom)
    );
    randomBtn->setPosition({ (winSize.width / 2) - 23, (winSize.height / 2) - 72});
    menu->addChild(randomBtn);

    auto buttonSprite = ButtonSprite::create("More", (int)(90*0.5), true, "bigFont.fnt", "GJ_button_01.png", 44*0.5f, 0.5f);
    auto buttonButton = CCMenuItemSpriteExtra::create(
        buttonSprite,
        this,
        menu_selector(DailyViewLayer::onMore)
    );
    buttonButton->setSizeMult(1.2f);
    buttonButton->setPosition({- winSize.width / 2, - winSize.height / 2});
    buttonButton->setAnchorPoint({0,0});
    menu->addChild(buttonButton);

    loadPage(0);
    return true;
}

void DailyViewLayer::loadPage(unsigned int page){

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    if(m_listLayer != nullptr) m_listLayer->removeFromParentAndCleanup(true);

    m_page = page;
    CCArray* displayedLevels = CCArray::create();
    //TODO: can we clone this by passing an iterator or something like that
    const unsigned int levelCount = levelsPerPage();
    unsigned int firstIndex = page * levelCount;
    unsigned int lastIndex = (page+1) * levelCount;

    for(unsigned int i = firstIndex; i < lastIndex; i++){
        auto levelObject = m_sortedLevels->objectAtIndex(i);
        if(i >= m_sortedLevels->count() || levelObject == nullptr) break;

        displayedLevels->addObject(levelObject);
    }

    auto listView = CvoltonListView<DailyCell>::create(displayedLevels, 356.f, 220.f);
    m_listLayer = GJListLayer::create(listView, m_isWeekly ? "Weekly Demons" : "Daily Levels", {191, 114, 62, 255}, 356.f, 220.f);
    m_listLayer->setPosition(winSize / 2 - m_listLayer->getScaledContentSize() / 2 - CCPoint(0,5));
    addChild(m_listLayer);

    if(page == 0) m_prevBtn->setVisible(false);
    else m_prevBtn->setVisible(true);

    if(m_sortedLevels->count() > lastIndex) m_nextBtn->setVisible(true);
    else m_nextBtn->setVisible(false);

    m_pageBtnSprite->setString(std::to_string(page+1).c_str());

    m_counter->setCString(CCString::createWithFormat("%i to %i of %i", firstIndex+1, (m_sortedLevels->count() >= lastIndex) ? lastIndex : m_sortedLevels->count(), m_sortedLevels->count())->getCString());
}

void DailyViewLayer::keyBackClicked() {
    setTouchEnabled(false);
    setKeypadEnabled(false);
    m_sortedLevels->release();
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}


void DailyViewLayer::onBack(CCObject* object) {
    keyBackClicked();
}

void DailyViewLayer::onPrev(CCObject* object) {
    loadPage(--m_page);
}

void DailyViewLayer::onNext(CCObject* object) {
    loadPage(++m_page);
}

void DailyViewLayer::onJumpToPageLayer(CCObject* sender){
    JumpToPageLayer::create(this)->show();
}

void DailyViewLayer::onRandom(CCObject* sender){
    loadPage(BetterInfo::randomNumber(0, m_sortedLevels->count() / levelsPerPage()));
}

void DailyViewLayer::onMore(CCObject* object) {
    auto searchObject = GJSearchObject::create(m_isWeekly ? SearchType::WeeklyVault : SearchType::DailyVault);
    auto browserLayer = LevelBrowserLayer::scene(searchObject);

    auto transitionFade = CCTransitionFade::create(0.5, browserLayer);

    CCDirector::sharedDirector()->pushScene(transitionFade);
}

CCScene* DailyViewLayer::scene(bool isWeekly) {
    auto layer = DailyViewLayer::create(isWeekly);
    auto scene = CCScene::create();
    scene->addChild(layer);
    return scene;
}

int DailyViewLayer::getPage() const{
    return m_page;
}

int DailyViewLayer::levelsPerPage() const{
    return (GameManager::sharedState()->getGameVariable("0093")) ? 20 : 10;
}

void DailyViewLayer::keyDown(enumKeyCodes key){
    switch(key){
        case KEY_Left:
        case CONTROLLER_Left:
            if(m_prevBtn->isVisible() == true) onPrev(nullptr);
            break;
        case KEY_Right:
        case CONTROLLER_Right:
            if(m_nextBtn->isVisible() == true) onNext(nullptr);
            break;
        default:
            CCLayer::keyDown(key);
    }
}