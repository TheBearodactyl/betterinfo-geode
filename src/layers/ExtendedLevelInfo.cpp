#include "ExtendedLevelInfo.h"
#include "UnregisteredProfileLayer.h"
#include "PaginatedFLAlert.h"
#include "../utils.hpp"
#include "../managers/BetterInfoStats.h"

#include <deque>
#include <algorithm>

ExtendedLevelInfo* ExtendedLevelInfo::create(GJGameLevel* level){
    auto ret = new ExtendedLevelInfo();
    if (ret && ret->init(level)) {
        //robert 1 :D
        ret->autorelease();
    } else {
        //robert -1
        delete ret;
        ret = nullptr;
    }
    return ret;
}

void ExtendedLevelInfo::onClose(cocos2d::CCObject* sender)
{
    m_level->release();
    setKeypadEnabled(false);
    removeFromParentAndCleanup(true);
}

void ExtendedLevelInfo::onCopyName(cocos2d::CCObject* sender)
{
    BetterInfo::copyToClipboard(m_level->m_levelName.c_str(), this);
}

void ExtendedLevelInfo::onCopyAuthor(cocos2d::CCObject* sender)
{
    BetterInfo::copyToClipboard(m_level->m_creatorName.c_str(), this);
}

void ExtendedLevelInfo::onCopyDesc(cocos2d::CCObject* sender)
{
    BetterInfo::copyToClipboard(m_level->getUnpackedLevelDescription().c_str(), this);
}

void ExtendedLevelInfo::onNext(cocos2d::CCObject* sender)
{
    loadPage(m_page+1);
}

void ExtendedLevelInfo::onPrev(cocos2d::CCObject* sender)
{
    loadPage(m_page-1);
}

void ExtendedLevelInfo::loadPage(int page) {
    this->m_page = page;
    if(page % 2 == 0) { 
        m_info->setString(m_primary);
        m_nextBtn->setVisible(true);
        m_prevBtn->setVisible(false);
    } else {
        m_info->setString(m_secondary);
        m_nextBtn->setVisible(false);
        m_prevBtn->setVisible(true);
    } 
}

std::string ExtendedLevelInfo::getGameVersionName(int version){
    if(version < 1 || version > 99) return std::string("NA");

    switch(version){
        case 10:
            return std::string("1.7");
        case 11:
            return std::string("Early 1.8");
    }

    std::stringstream contentStream;

    if(version > 17){
        double newVersion = (double)version / 10.0;
        contentStream << std::fixed << std::setprecision(1) << newVersion;
    }else{
        contentStream << "1." << (version-1);
    }
    return contentStream.str();
}

std::string ExtendedLevelInfo::stringDate(std::string date){
    if(date == "") return "NA";
    std::ostringstream stream;
    stream << date << " ago";
    return stream.str();
}

const char* ExtendedLevelInfo::boolString(bool value) {
    return value ? "True" : "False";
}

const char* ExtendedLevelInfo::getDifficultyIcon(int stars){
    switch(stars){
        case 1: 
            return "difficulty_auto_btn_001.png";
        case 2:
            return "difficulty_01_btn_001.png";
        case 3:
            return "difficulty_02_btn_001.png";
        case 4:
        case 5:
            return "difficulty_03_btn_001.png";
        case 6:
        case 7:
            return "difficulty_04_btn_001.png";
        case 8:
        case 9:
            return "difficulty_05_btn_001.png";
        case 10:
            return "difficulty_06_btn_001.png";
        default:
            return "difficulty_00_btn_001.png";
    }
}

std::string ExtendedLevelInfo::passwordString(int password){
    if(password == 0) return "NA";
    if(password == 1) return "Free Copy";
    if(password >= 10000 && password <= 19999) return std::to_string(password - 10000);
    if(password >= 1000000 && password <= 1999999) return std::to_string(password - 1000000);
    return "Invalid (" + std::to_string(password) + ")";
}

std::string ExtendedLevelInfo::zeroIfNA(int value){
    if(value == 0) return "NA";
    return std::to_string(value);
}

std::string ExtendedLevelInfo::workingTime(int value){
    if(value <= 0) return "NA";

    int hours = value / 3600;
    int minutes = (value % 3600) / 60;
    int seconds = value % 60;

    std::ostringstream stream;
    if(hours > 0) stream << hours << "h ";
    if(minutes > 0) stream << minutes << "m ";
    stream << seconds << "s";

    return stream.str();
}

std::string ExtendedLevelInfo::printableProgress(std::string personalBests, int percentage){

    std::stringstream bestsStream(personalBests);
    std::string currentBest;
    std::deque<int> progresses;
    while(getline(bestsStream, currentBest, ',')){
        try {
            progresses.push_front(std::stoi(currentBest));
            //contentStream << percentage << "% ";
        }catch(...){}
    }
    std::string printable;
    //std::reverse(std::begin(progresses), std::end(progresses));
    for(auto i : progresses){
        //contentStream << percentage << "% ";
        //printable.insert(0, std::format("{}% ", percentage));
        printable = std::to_string(percentage) + "% " + printable;
        percentage -= i;
    }

    return printable;
}

bool ExtendedLevelInfo::init(GJGameLevel* level){
    bool init = createBasics({440.0f, 290.0f}, menu_selector(ExtendedLevelInfo::onClose));
    if(!init) return false;

    level->retain();
    this->m_level = level;

    auto levelName = CCLabelBMFont::create(m_level->m_levelName.c_str(), "bigFont.fnt");
    auto levelNameBtn = CCMenuItemSpriteExtra::create(
        levelName,
        this,
        menu_selector(ExtendedLevelInfo::onCopyName)
    );
    m_buttonMenu->addChild(levelNameBtn);
    levelNameBtn->setPosition({0,125});

    std::ostringstream userNameText;
    userNameText << "By " << std::string(m_level->m_creatorName);
    auto userName = CCLabelBMFont::create(userNameText.str().c_str(), "goldFont.fnt");
    userName->setScale(0.8f);
    auto userNameBtn = CCMenuItemSpriteExtra::create(
        userName,
        this,
        menu_selector(ExtendedLevelInfo::onCopyAuthor)
    );
    userNameBtn->setPosition({0,99});
    m_buttonMenu->addChild(userNameBtn);

    cocos2d::extension::CCScale9Sprite* descBg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    descBg->setContentSize({340,55});
    descBg->setColor({130,64,33});
    m_buttonMenu->addChild(descBg, -1);
    descBg->setPosition({0,52});

    auto descText = BetterInfo::fixNullByteCrash(BetterInfo::fixColorCrashes(m_level->getUnpackedLevelDescription()));
    size_t descLength = descText.length();
    float descDelimiter = 1;
    if(descLength > 140) descLength = 140;
    if(descLength > 70) descDelimiter = ((((140 - descLength) / 140) * 0.3f) + 0.7f);
    auto description = TextArea::create(descText, "chatFont.fnt", 1, 295 / descDelimiter, {0.5f,0.5f}, 20, false);
    description->setScale(descDelimiter);
    description->setAnchorPoint({1,1});
    description->setPosition( ( (description->getContentSize() / 2 ) * descDelimiter ) + (CCPoint(340,55) / 2) );
    auto descSprite = CCSprite::create();
    descSprite->addChild(description);
    descSprite->setContentSize({340,55});
    auto descBtn = CCMenuItemSpriteExtra::create(
        descSprite,
        this,
        menu_selector(ExtendedLevelInfo::onCopyDesc)
    );
    descBtn->setPosition({0,52});
    m_buttonMenu->addChild(descBtn);

    cocos2d::extension::CCScale9Sprite* infoBg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    infoBg->setContentSize({340,148});
    //infoBg->setColor({130,64,33});
    //infoBg->setColor({191,114,62});
    infoBg->setColor({123,60,31});
    m_buttonMenu->addChild(infoBg, -1);
    infoBg->setPosition({0,-57});

    //std::string levelString(ZipUtils::base64URLDecode(m_level->levelString));
    int levelPassword = m_level->m_password;
    std::ostringstream infoText;
    infoText << "\n<cj>Uploaded</c>: " << stringDate(m_level->m_uploadDate)
        << "\n<cj>Updated</c>: " << stringDate(m_level->m_updateDate)
        //<< "\n<cy>Stars Requested</c>: " << m_level->m_starsRequested
        << "\n<cg>Original</c>: " << zeroIfNA(m_level->m_originalLevel)
        //<< "\n<cg>Feature score</c>: " << zeroIfNA(m_level->m_featured)
        << "\n<cy>Game Version</c>: " << getGameVersionName(m_level->m_gameVersion)
        //<< "\nFeature Score</c>: " << m_level->m_featured
        << "\n<co>Password</c>: " << passwordString(levelPassword)
        << "\n<cr>In Editor</c>: " << workingTime(m_level->m_workingTime)
        << "\n<cr>Editor (C)</c>: " << workingTime(m_level->m_workingTime2);

    m_primary = infoText.str();
    std::string levelString(BetterInfo::decodeBase64Gzip(m_level->m_levelString));
    size_t objectsEstimated = std::count(levelString.begin(), levelString.end(), ';');
    infoText.str("");
    infoText << "\n<cj>Objects</c>: " << zeroIfNA(m_level->m_objectCount)
        << "\n<cg>Objects (est.)</c>: " << zeroIfNA(objectsEstimated) //i have no idea what the 0 and 11 mean, i just copied them from PlayLayer::init
        << "\n<cy>Feature Score</c>: " << zeroIfNA(m_level->m_featured)
        << "\n<co>Two-player</c>: " << boolString(m_level->m_twoPlayerMode)
        << "\n<cr>Size</c>: " << BetterInfo::fileSize(m_level->m_levelString.size()) << " / " << BetterInfo::fileSize(levelString.size());
    ;

    m_secondary = infoText.str();

    /*infoText << "<cg>Total Attempts</c>: " << m_level->attempts
        << "\n<cl>Total Jumps</c>: " << m_level->jumps
        << "\n<co>Clicks (best att.)</c>: " << m_level->clicks // the contents of this variable make no sense to me
        << "\n<co>Time (best att.)</c>: " << ExtendedLevelInfo::workingTime(m_level->attemptTime) // the contents of this variable make no sense to me
        //<< "\n<co>Is legit</c>: " << m_level->isCompletionLegitimate // the contents of this variable make no sense to me
        << "\n<cp>Normal</c>: " << m_level->normalPercent
        << "%\n<co>Practice</c>: " << m_level->practicePercent << "%";

    if(m_level->orbCompletion != m_level->newNormalPercent2) infoText << "\n<cj>2.1 Normal</c>: " << m_level->orbCompletion << "%";
    if(m_level->newNormalPercent2 != m_level->normalPercent) infoText << "\n<cr>2.11 Normal</c>: " << m_level->newNormalPercent2 << "%";*/

    m_info = TextArea::create(infoText.str(), "chatFont.fnt", 1, 170, {0,1}, 20, false);
    m_info->setPosition({-160.5,26});
    //m_info->setPosition({-160.5,10});
    m_info->setAnchorPoint({0,1});
    m_info->setScale(0.925f);
    m_buttonMenu->addChild(m_info);

    /*std::ostringstream progressText;
    if(m_level->personalBests != "") progressText << "\n\n<cy>Progresses</c>:\n" << printableProgress(m_level->personalBests, m_level->newNormalPercent2);
    auto progress = TextArea::create("chatFont.fnt", false, progressText.str(), 0.8f, 130, 20, {0,1});
    progress->setPosition({12,50});
    progress->setAnchorPoint({0,1});
    m_buttonMenu->addChild(progress);*/

    /*std::ostringstream uploadedText;
    uploadedText << "Uploaded: " << m_level->uploadDate << " ago";
    createTextLabel(uploadedText.str(), {0,0}, 0.5f, m_buttonMenu);*/

    createTextLabel("Requested Rate:", {88,-1}, 0.5f, m_buttonMenu);

    auto diffSprite = CCSprite::createWithSpriteFrameName(getDifficultyIcon(m_level->m_starsRequested));
    diffSprite->setPosition({88,-57});
    m_buttonMenu->addChild(diffSprite, 1);

    if(m_level->m_starsRequested > 0){
        auto featureSprite = CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png");
        featureSprite->setPosition({88,-57});
        m_buttonMenu->addChild(featureSprite);
        //infoSprite->setScale(0.7f);

        auto starsLabel = createTextLabel(std::to_string(m_level->m_starsRequested), {88, -87}, 0.4f, m_buttonMenu);
        starsLabel->setAnchorPoint({1,0.5});

        auto diffSprite = CCSprite::createWithSpriteFrameName("star_small01_001.png");
        diffSprite->setPosition({95,-87});
        m_buttonMenu->addChild(diffSprite);
    }

    /*
        thanks to Alphalaneous for quick UI improvement concept lol
    */

    auto separator = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    separator->setPosition({6,-57});
    separator->setScaleX(0.3f);
    separator->setScaleY(1);
    separator->setOpacity(100);
    separator->setRotation(90);
    m_buttonMenu->addChild(separator);

    /*
        next/prev page btn
    */
    auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    prevSprite->setScale(.8f);
    m_prevBtn = CCMenuItemSpriteExtra::create(
        prevSprite,
        this,
        menu_selector(ExtendedLevelInfo::onPrev)
    );
    m_prevBtn->setPosition({-195,-53});
    m_buttonMenu->addChild(m_prevBtn);

    auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextSprite->setFlipX(true);
    nextSprite->setScale(.8f);
    m_nextBtn = CCMenuItemSpriteExtra::create(
        nextSprite,
        this,
        menu_selector(ExtendedLevelInfo::onNext)
    );
    m_nextBtn->setPosition({195,-53});
    m_buttonMenu->addChild(m_nextBtn);

    loadPage(0);

    return true;
}

CCLabelBMFont* ExtendedLevelInfo::createTextLabel(const std::string text, const CCPoint& position, const float scale, CCNode* menu, const char* font){
    CCLabelBMFont* bmFont = CCLabelBMFont::create(text.c_str(), font);
    bmFont->setPosition(position);
    bmFont->setScale(scale);
    menu->addChild(bmFont);
    return bmFont;
}

void ExtendedLevelInfo::showProgressDialog(GJGameLevel* level){

    if(level == nullptr) return;


    if(level->m_levelID == -1){
        FLAlertLayer::create(nullptr, "It's a secret...",  "The <cr>darkness</c> lingers. Be careful who you trust...\nThere might be an <c_>impostor</c> among us.", "OK", nullptr, 360)->show();
        return;
    }

    std::ostringstream contentStream, secondStream;
    contentStream << "<cg>Total Attempts</c>: " << level->m_attempts
        << "\n<cl>Total Jumps</c>: " << level->m_jumps
        << "\n<cy>Clicks (best att.)</c>: " << level->m_clicks
        << "\n<cr>Time (best att.)</c>: " << ExtendedLevelInfo::workingTime(level->m_attemptTime);

    if(level->m_levelType != GJLevelType::Editor){
        auto stats = BetterInfoStats::sharedState();

        contentStream << "\n<cp>Normal</c>: " << level->m_normalPercent
        << "%\n<co>Practice</c>: " << level->m_practicePercent << "%";
        if(level->m_orbCompletion != level->m_normalPercent) contentStream << "\n<cj>Orbs</c>: " << level->m_orbCompletion << "%";
        if(level->m_newNormalPercent2 != level->m_orbCompletion) contentStream << "\n<cr>Leaderboard</c>: " << level->m_newNormalPercent2 << "%";

        auto normalAttempts = stats->getAttempts(level, false);
        auto practiceAttempts = stats->getAttempts(level, true);
        secondStream << "<cp>Attempts (normal)</c>: " << normalAttempts << "\n";
        secondStream << "<co>Attempts (practice)</c>: " << practiceAttempts << "\n";
        if((normalAttempts + practiceAttempts) != level->m_attempts) secondStream << "<cy>Attempts (unknown)</c>: " << (level->m_attempts - practiceAttempts - normalAttempts) << "\n";
        secondStream << "\n";
        if(stats->getPlay(level, false) != 0) secondStream << "<cg>First played</c>: " << BetterInfo::timeToString(stats->getPlay(level, false)) << "\n";
        if(stats->getPlay(level, true) != 0) secondStream << "<cl>Last played</c>: " << BetterInfo::timeToString(stats->getPlay(level, true)) << "\n";
        if(stats->getCompletion(level, false) > 0) secondStream << "<cp>Completed</c>: " << BetterInfo::timeToString(stats->getCompletion(level, false)) << "\n";
        if(stats->getCompletion(level, true) > 0) secondStream << "<co>Completed (practice)</c>: " << BetterInfo::timeToString(stats->getCompletion(level, true)) << "\n";
    }else{
        contentStream << "\n<cp>Objects</c>: " << level->m_objectCount
            << "\n<cr>In Editor</c>: " << ExtendedLevelInfo::workingTime(level->m_workingTime) << "\n";
        if(level->m_workingTime2) contentStream << "<cr>Editor (C)</c>: " << ExtendedLevelInfo::workingTime(level->m_workingTime2) << "\n";
        ;
    }

    std::string progresses;
    if(std::string(level->m_personalBests) != ""){
        progresses = ExtendedLevelInfo::printableProgress(level->m_personalBests, level->m_newNormalPercent2);
        contentStream << "\n\n<cy>Progresses</c>: " << progresses;
    }

    std::vector<std::string> contents;
    contents.push_back(contentStream.str());
    if(!(secondStream.str().empty())) contents.push_back(secondStream.str());

    /*float dialogWidth = 350;
    if(contentStream.str().length() > 250) dialogWidth = 400;*/
    //FLAlertLayer::create(nullptr, level->m_levelName.c_str(), "OK", nullptr, dialogWidth, contentStream.str().length() > 250, 300, contentStream.str())->m_show();
    PaginatedFLAlert::create(level->m_levelName, contents)->show();
}