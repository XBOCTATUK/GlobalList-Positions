#include "./GlobalListLayer/GlobalListLayer.hpp"
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>

using namespace geode::prelude;

template <typename T>
T GetJsonValue(const matjson::Value& parent, const std::string key, T defaultValue) {
    if (!parent.contains(key) || parent[key].isNull())
        return defaultValue;
    auto result = parent[key].as<T>();

    if (result.isOk())
        return result.unwrap();
    else
        return defaultValue;
}

matjson::Value positionsCache;

class $modify(MyLevelCell, LevelCell) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
    };

    void loadFromLevel(GJGameLevel * level) {
        LevelCell::loadFromLevel(level);
        auto compactLists = Loader::get()->getLoadedMod("cvolton.compact_lists");

        if (level->m_demonDifficulty == int(DemonDifficultyType::ExtremeDemon) ||
            level->m_demonDifficulty == int(DemonDifficultyType::InsaneDemon) || (
                level->getAverageDifficulty() == int(GJDifficulty::Insane) && level->m_stars == 0)) {
            auto levelCellMain = this->getChildByID("main-layer");
            auto downloadIcon = levelCellMain->getChildByID("downloads-icon");
            auto downloadLabel = levelCellMain->getChildByID("downloads-label");
            auto likesIcon = levelCellMain->getChildByID("likes-icon");
            auto likesLabel = levelCellMain->getChildByID("likes-label");
            auto lengthIcon = levelCellMain->getChildByID("length-icon");
            auto lengthLabel = levelCellMain->getChildByID("length-label");
            auto orbIcon = levelCellMain->getChildByID("orbs-icon");
            auto orbLabel = levelCellMain->getChildByID("orbs-label");

            float globalListIconX = 0.0f;
            if (orbLabel)
                globalListIconX = orbLabel->getScaleX() * orbLabel->getContentWidth() + orbLabel->getPositionX() +
                (downloadIcon->getPositionX() - downloadIcon->getContentWidth() * downloadIcon->getScaleX() / 2.0f -
                    lengthLabel->getContentWidth() * lengthLabel->getScaleX() - lengthLabel->getPositionX());
            else
                globalListIconX = likesLabel->getScaleX() * likesLabel->getContentWidth() + likesLabel->getPositionX() +
                (downloadIcon->getPositionX() - downloadIcon->getContentWidth() * downloadIcon->getScaleX() / 2.0f -
                    lengthLabel->getContentWidth() * lengthLabel->getScaleX() - lengthLabel->getPositionX());

            auto globalListIcon = CCSprite::create("global-list.png"_spr);
            globalListIcon->setScale(0.45f);
            globalListIcon->setContentSize({ 23.5f, 23.5f });
            globalListIcon->setAnchorPoint({ 0.5f, 0.5f });
            globalListIcon->setPosition({ globalListIconX, 13.0f });
            globalListIcon->refreshTextureRect();
            globalListIcon->setID("global-list-icon"_spr);
            levelCellMain->addChild(globalListIcon);

            float globalListLabelX = globalListIconX + globalListIcon->getContentWidth() / 2;

            auto globalListLabel = CCLabelBMFont::create("...", "bigFont.fnt");
            globalListLabel->setScale(0.4f);
            globalListLabel->setAnchorPoint({ 0.0f, 0.5f });
            globalListLabel->setPosition({ globalListLabelX, 14.0f });
            globalListLabel->setID("global-list-label"_spr);
            levelCellMain->addChild(globalListLabel);

            float gap = (likesIcon->getPositionX() - likesIcon->getContentWidth() * likesIcon->getScaleX() / 2.0f) -
                (downloadLabel->getContentWidth() * downloadLabel->getScaleX() + downloadLabel->getPositionX());
            bool gapFlag = false;
            
            if (compactLists && compactLists->isEnabled() && compactLists->getSettingValue<bool>("enable-compact-lists")) {
                globalListIcon->setScale(0.3f);
                globalListIcon->setPositionY(8.5f);
                globalListLabel->setScale(0.3f);
                globalListLabel->setPosition(globalListLabel->getPositionX() - 2.0f, 8.5f);
            }
            else if (globalListLabel->getPositionX() > 310.0f) {
                gapFlag = true;
                downloadIcon->setPositionX(downloadIcon->getPositionX() - gap * 0.6f);
                downloadLabel->setPositionX(downloadLabel->getPositionX() - gap * 0.6f);
                likesIcon->setPositionX(likesIcon->getPositionX() - gap * 1.2f);
                likesLabel->setPositionX(likesLabel->getPositionX() - gap * 1.2f);
                orbIcon->setPositionX(orbIcon->getPositionX() - gap * 1.8f);
                orbLabel->setPositionX(orbLabel->getPositionX() - gap * 1.8f);
                globalListIcon->setPositionX(globalListIcon->getPositionX() - gap * 2.4f);
                globalListLabel->setPositionX(globalListLabel->getPositionX() - gap * 2.4f);
            }

            getPos(gap, gapFlag);
        }
    }

    void getPos(float gap, bool gapFlag) {
        int levelID = m_level->m_levelID.value();
        auto globalListIcon = static_cast<CCLabelBMFont*>(this->getChildByIDRecursive("global-list-icon"_spr));
        auto globalListLabel = static_cast<CCLabelBMFont*>(this->getChildByIDRecursive("global-list-label"_spr));
        auto levelCellMain = this->getChildByID("main-layer");
        auto downloadIcon = levelCellMain->getChildByID("downloads-icon");
        auto downloadLabel = levelCellMain->getChildByID("downloads-label");
        auto likesIcon = levelCellMain->getChildByID("likes-icon");
        auto likesLabel = levelCellMain->getChildByID("likes-label");
        auto lengthIcon = levelCellMain->getChildByID("length-icon");
        auto lengthLabel = levelCellMain->getChildByID("length-label");
        auto orbIcon = levelCellMain->getChildByID("orbs-icon");
        auto orbLabel = levelCellMain->getChildByID("orbs-label");

        if (positionsCache.contains(std::to_string(levelID))) {
            int place = GetJsonValue<int>(positionsCache, std::to_string(levelID), 0);
            std::string globalListLabellStr = "#" + std::to_string(place);
            globalListLabel->setString(globalListLabellStr.c_str());
        }
        else {
            std::string url = "https://api.demonlist.org/levels/classic?level_id=" + std::to_string(levelID);
            auto req = web::WebRequest();
            m_fields->m_listener.bind([levelID, globalListIcon, globalListLabel, gap, gapFlag, this](web::WebTask::Event* e) {
                if (auto res = e->getValue()) {
                    if (!res->ok()) {
                        log::error("Request error: {}", res->code());
                        globalListLabel->setString("N/A");
                    }
                    else {
                        auto data = res->json();
                        matjson::Value json = data.unwrapOr(matjson::Value::object());

                        if (!json.contains("data") || !json["data"].isArray() || json["data"].size() == 0) {
                            globalListIcon->setVisible(false);
                            globalListLabel->setVisible(false);

                            if (gapFlag) {
                                auto levelCellMain = this->getChildByID("main-layer");
                                auto downloadIcon = levelCellMain->getChildByID("downloads-icon");
                                auto downloadLabel = levelCellMain->getChildByID("downloads-label");
                                auto likesIcon = levelCellMain->getChildByID("likes-icon");
                                auto likesLabel = levelCellMain->getChildByID("likes-label");
                                auto orbIcon = levelCellMain->getChildByID("orbs-icon");
                                auto orbLabel = levelCellMain->getChildByID("orbs-label");

                                downloadIcon->setPositionX(downloadIcon->getPositionX() + gap * 0.6f);
                                downloadLabel->setPositionX(downloadLabel->getPositionX() + gap * 0.6f);
                                likesIcon->setPositionX(likesIcon->getPositionX() + gap * 1.2f);
                                likesLabel->setPositionX(likesLabel->getPositionX() + gap * 1.2f);
                                if (orbIcon) {
                                    orbIcon->setPositionX(orbIcon->getPositionX() + gap * 1.8f);
                                    orbLabel->setPositionX(orbLabel->getPositionX() + gap * 1.8f);
                                    globalListIcon->setPositionX(globalListIcon->getPositionX() + gap * 2.4f);
                                    globalListLabel->setPositionX(globalListLabel->getPositionX() + gap * 2.4f);
                                }
                                globalListIcon->setPositionX(globalListIcon->getPositionX() + gap * 1.8f);
                                globalListLabel->setPositionX(globalListLabel->getPositionX() + gap * 1.8f);
                            }

                            return;
                        }
                        const matjson::Value& levelData = json["data"][0];

                        int place = GetJsonValue<int>(levelData, "place", 0);

                        if (place != 0) {
                            positionsCache[std::to_string(levelID)] = place;

                            std::string globalListLabellStr = "#" + std::to_string(place);
                            globalListLabel->setString(globalListLabellStr.c_str());
                        }
                        else {
                            globalListLabel->setString("N/A");
                        }
                    }
                }
                else if (e->isCancelled()) {
                    log::warn("Request is canceled");
                    globalListLabel->setString("N/A");
                }
                });
            auto task = req.get(url);
            m_fields->m_listener.setFilter(task);
        }
    }
};

class $modify(LevelInfoLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
    };

    bool init(GJGameLevel * level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        if (level->m_demonDifficulty == int(DemonDifficultyType::ExtremeDemon) ||
            level->m_demonDifficulty == int(DemonDifficultyType::InsaneDemon) || (
                level->getAverageDifficulty() == int(GJDifficulty::Insane) && level->m_stars == 0)) {
            float globalListIconY = 0.0f;

            auto downloadIcon = this->getChildByID("downloads-icon");
            auto downloadLabel = this->getChildByID("downloads-label");
            auto likesIcon = this->getChildByID("likes-icon");
            auto likesLabel = this->getChildByID("likes-label");
            auto lengthIcon = this->getChildByID("length-icon");
            auto lengthLabel = this->getChildByID("length-label");
            auto exactLengthLabel = this->getChildByID("exact-length-label");
            auto orbIcon = this->getChildByID("orbs-icon");
            auto orbLabel = this->getChildByID("orbs-label");

            downloadIcon->setPositionY(downloadIcon->getPositionY() + 14.0f);
            downloadLabel->setPositionY(downloadLabel->getPositionY() + 14.0f);
            likesIcon->setPositionY(likesIcon->getPositionY() + 14.0f);
            likesLabel->setPositionY(likesLabel->getPositionY() + 14.0f);
            lengthIcon->setPositionY(lengthIcon->getPositionY() + 14.0f);
            lengthLabel->setPositionY(lengthLabel->getPositionY() + 14.0f);
            exactLengthLabel->setPositionY(exactLengthLabel->getPositionY() + 14.0f);
            orbIcon->setPositionY(orbIcon->getPositionY() + 14.0f);
            orbLabel->setPositionY(orbLabel->getPositionY() + 14.0f);

            if (orbIcon->isVisible())
                globalListIconY = orbIcon->getPositionY() - (lengthIcon->getPositionY() - orbIcon->getPositionY());
            else
                globalListIconY = lengthIcon->getPositionY() - (likesIcon->getPositionY() - lengthIcon->getPositionY());

            float globalListIconX = lengthIcon->getPositionX() + lengthIcon->getContentWidth() / 2.0f;

            auto globalListIcon = CCSprite::create("global-list.png"_spr);
            globalListIcon->setContentSize({ 23.5f, 23.5f });
            globalListIcon->setScale(0.85f);
            globalListIcon->setAnchorPoint({ 0.5f, 0.5f });
            globalListIcon->setPosition({ globalListIconX, globalListIconY });
            globalListIcon->refreshTextureRect();
            globalListIcon->setID("global-list-icon"_spr);
            this->addChild(globalListIcon);

            auto globalListLabel = CCLabelBMFont::create("...", "bigFont.fnt");
            globalListLabel->setScale(0.5f);
            globalListLabel->setAnchorPoint({ 0.0f, 0.5f });
            globalListLabel->setPosition({ lengthLabel->getPositionX(), globalListIconY });
            globalListLabel->setID("global-list-label"_spr);
            this->addChild(globalListLabel);

            getPos();
        }

        return true;
    }

    void getPos() {
        int levelID = m_level->m_levelID.value();
        auto globalListIcon = static_cast<CCLabelBMFont*>(this->getChildByIDRecursive("global-list-icon"_spr));
        auto globalListLabel = static_cast<CCLabelBMFont*>(this->getChildByIDRecursive("global-list-label"_spr));

        if (positionsCache.contains(std::to_string(levelID))) {
            int place = GetJsonValue<int>(positionsCache, std::to_string(levelID), 0);
            std::string globalListLabellStr = "#" + std::to_string(place);
            globalListLabel->setString(globalListLabellStr.c_str());
        }
        else {
            std::string url = "https://api.demonlist.org/levels/classic?level_id=" + std::to_string(levelID);
            auto req = web::WebRequest();
            m_fields->m_listener.bind([levelID, globalListIcon, globalListLabel, this](web::WebTask::Event* e) {
                if (auto res = e->getValue()) {
                    if (!res->ok()) {
                        log::error("Request error: {}", res->code());
                        globalListLabel->setString("N/A");
                    }
                    else {
                        auto data = res->json();
                        matjson::Value json = data.unwrapOr(matjson::Value::object());

                        if (!json.contains("data") || !json["data"].isArray() || json["data"].size() == 0) {
                            globalListIcon->setVisible(false);
                            globalListLabel->setVisible(false);

                            auto downloadIcon = this->getChildByID("downloads-icon");
                            auto downloadLabel = this->getChildByID("downloads-label");
                            auto likesIcon = this->getChildByID("likes-icon");
                            auto likesLabel = this->getChildByID("likes-label");
                            auto lengthIcon = this->getChildByID("length-icon");
                            auto lengthLabel = this->getChildByID("length-label");
                            auto exactLengthLabel = this->getChildByID("exact-length-label");
                            auto orbIcon = this->getChildByID("orbs-icon");
                            auto orbLabel = this->getChildByID("orbs-label");

                            downloadIcon->setPositionY(downloadIcon->getPositionY() - 14.0f);
                            downloadLabel->setPositionY(downloadLabel->getPositionY() - 14.0f);
                            likesIcon->setPositionY(likesIcon->getPositionY() - 14.0f);
                            likesLabel->setPositionY(likesLabel->getPositionY() - 14.0f);
                            lengthIcon->setPositionY(lengthIcon->getPositionY() - 14.0f);
                            lengthLabel->setPositionY(lengthLabel->getPositionY() - 14.0f);
                            exactLengthLabel->setPositionY(exactLengthLabel->getPositionY() - 14.0f);
                            orbIcon->setPositionY(orbIcon->getPositionY() - 14.0f);
                            orbLabel->setPositionY(orbLabel->getPositionY() - 14.0f);

                            return;
                        }
                        const matjson::Value& levelData = json["data"][0];

                        int place = GetJsonValue<int>(levelData, "place", 0);

                        if (place != 0) {
                            positionsCache[std::to_string(levelID)] = place;

                            std::string globalListLabellStr = "#" + std::to_string(place);
                            globalListLabel->setString(globalListLabellStr.c_str());
                        }
                        else {
                            globalListLabel->setString("N/A");
                        }
                    }
                }
                else if (e->isCancelled()) {
                    log::warn("Request is canceled");
                    globalListLabel->setString("N/A");
                }
                });
            auto task = req.get(url);
            m_fields->m_listener.setFilter(task);
        }
    }
};

class $modify(MyLevelSearchLayer, LevelSearchLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
        LevelBrowserLayer* m_currentBrowserLayer = nullptr;
        std::vector<int> m_globalListLevels;
    };

    bool init(int p0) {
        if (!LevelSearchLayer::init(p0)) return false;

        auto filterMenu = this->getChildByID("other-filter-menu");

        auto globalListSpr = CircleButtonSprite::createWithSprite("global-list.png"_spr);
        globalListSpr->getTopNode()->setScale(1.1f);
        globalListSpr->setScale(0.8f);
        auto globalListBtn = CCMenuItemSpriteExtra::create(globalListSpr, this, menu_selector(MyLevelSearchLayer::onBtn));
        globalListBtn->setID("global-list-button");
        filterMenu->addChild(globalListBtn);
        filterMenu->updateLayout();

        return true;
    }

    void onBtn(CCObject* sender) {
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, GlobalListLayer::scene()));
    }
};