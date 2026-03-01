#include "./GlobalListLayer/GlobalListLayer.hpp"
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/modify/CCContentLayer.hpp>

void removePlacement(const int levelID, CCNode* gdlLabel, CCNode* gdlIcon, const std::unordered_map<CCNode*, float> origPositions, const bool onLevelCell) {
    g_levelsWithoutPlacement[levelID] = true;
    if (gdlLabel) gdlLabel->setVisible(false);
    if (gdlIcon) gdlIcon->setVisible(false);
    for (auto& [node, pos] : origPositions) if (node) onLevelCell ? node->setPositionX(pos) : node->setPositionY(pos);
}

class $modify(MyLevelCell, LevelCell) {
    struct Fields {
        TaskHolder<web::WebResponse> m_listener;
        std::unordered_map<CCNode*, float> m_origPositions;
    };

    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);
        if (!level || level->m_levelType == GJLevelType::Main || level->m_levelType == GJLevelType::Editor || g_levelsWithoutPlacement.contains(level->m_levelID.value())) return;

        auto levelCellMain = this->m_mainLayer;

        bool isDemon =
            level->m_demonDifficulty == static_cast<int>(DemonDifficultyType::ExtremeDemon) ||
            level->m_demonDifficulty == static_cast<int>(DemonDifficultyType::InsaneDemon);

        if (isDemon || level->m_stars == 0) {
            auto downloadIcon = levelCellMain->getChildByID("downloads-icon");
            auto downloadLabel = levelCellMain->getChildByID("downloads-label");
            auto likesIcon = levelCellMain->getChildByID("likes-icon");
            auto likesLabel = levelCellMain->getChildByID("likes-label");
            auto lengthIcon = levelCellMain->getChildByID("length-icon");
            auto lengthLabel = levelCellMain->getChildByID("length-label");
            auto orbIcon = levelCellMain->getChildByID("orbs-icon");
            auto orbLabel = levelCellMain->getChildByID("orbs-label");

            float gdlIconX = 0.0f;
            float downloadIconPos = downloadIcon->getPositionX() - downloadIcon->getContentWidth() * downloadIcon->getScaleX() / 2.0f;
            float lengthLabelPos = lengthLabel->getPositionX() + lengthLabel->getContentWidth() * lengthLabel->getScaleX();
            float likesLabelPos = likesLabel->getScaleX() * likesLabel->getContentWidth() + likesLabel->getPositionX();

            if (orbLabel) {
                float orbLabelPos = orbLabel->getScaleX() * orbLabel->getContentWidth() + orbLabel->getPositionX();
                gdlIconX = orbLabelPos + (downloadIconPos - lengthLabelPos);
            }
            else gdlIconX = likesLabelPos + (downloadIconPos - lengthLabelPos);

            auto gdlIcon = CCSprite::create("global-list.png"_spr);
            gdlIcon->setScale(0.55f);
            gdlIcon->setContentSize({ 23.5f, 23.5f });
            gdlIcon->setAnchorPoint({ 0.5f, 0.5f });
            gdlIcon->setPosition({ gdlIconX + gdlIcon->getContentWidth() * 0.45f / 2.0f, 14.0f });
            gdlIcon->refreshTextureRect();
            gdlIcon->setID("gdl-icon"_spr);
            levelCellMain->addChild(gdlIcon);

            float gdlLabelX = gdlIconX + gdlIcon->getContentWidth() / 2 + 3.0f;

            auto gdlLabel = CCLabelBMFont::create("...", "bigFont.fnt");
            gdlLabel->setScale(0.4f);
            gdlLabel->setAnchorPoint({ 0.0f, 0.5f });
            gdlLabel->setPosition({ gdlLabelX, 14.0f });
            gdlLabel->setID("gdl-label"_spr);
            levelCellMain->addChild(gdlLabel);

            std::unordered_map<CCNode*, float>& origPositions = m_fields->m_origPositions;

            if (this->m_compactView) {
                gdlIcon->setScale(0.3f);
                gdlIcon->setPositionY(8.5f);
                gdlLabel->setScale(0.3f);
                gdlLabel->setPosition(gdlLabel->getPositionX() - 2.0f, 8.5f);
            }
            else if (gdlLabel->getPositionX() + 50.0f > 356.0f) {
                float gap = ((gdlLabel->getPositionX() + 50.0f) - 356.0f) / (orbIcon ? 4.0f : 3.0f);

                origPositions[downloadIcon] = downloadIcon->getPositionX();
                origPositions[downloadLabel] = downloadLabel->getPositionX();
                origPositions[likesIcon] = likesIcon->getPositionX();
                origPositions[likesLabel] = likesLabel->getPositionX();
                if (orbIcon) origPositions[orbIcon] = orbIcon->getPositionX();
                if (orbLabel) origPositions[orbLabel] = orbLabel->getPositionX();
                origPositions[gdlIcon] = gdlIcon->getPositionX();
                origPositions[gdlLabel] = gdlLabel->getPositionX();
                downloadIcon->setPositionX(downloadIcon->getPositionX() - gap);
                downloadLabel->setPositionX(downloadLabel->getPositionX() - gap);
                likesIcon->setPositionX(likesIcon->getPositionX() - gap * 2.0f);
                likesLabel->setPositionX(likesLabel->getPositionX() - gap * 2.0f);
                if (orbIcon) orbIcon->setPositionX(orbIcon->getPositionX() - gap * 3.0f);
                if (orbLabel) orbLabel->setPositionX(orbLabel->getPositionX() - gap * 3.0f);
                gdlIcon->setPositionX(gdlIcon->getPositionX() - gap * (orbIcon ? 4.0f : 3.0f));
                gdlLabel->setPositionX(gdlLabel->getPositionX() - gap * (orbIcon ? 4.0f : 3.0f));
            }

            getPlacement(level->m_levelID.value());
        }
    }

    void getPlacement(int levelID) {
        if (g_placementsCache.contains(levelID)) {
            int placement = g_placementsCache[levelID];
            updatePlacement(placement);
        }
        else {
            m_fields->m_listener.spawn(web::WebRequest().get(fmt::format("https://api.demonlist.org/level/classic/get?ingame_id={}", levelID)),
            [this, levelID](web::WebResponse value) {
                int placement = -1;
                if (!value.ok()) {
                    if (value.code() == 404) placement = 0;
                    else placement = -1;
                }
                else {
                    auto data = value.json();
                    auto json = data.ok().value();

                    auto levelData = json["data"];

                    placement = levelData["placement"].asInt().ok().value();

                    if (placement > 0) {
                        g_placementsCache[levelID] = placement;
                    }
                    else placement = -1;
                }

                updatePlacement(placement);
            });
        }
    }

    void updatePlacement(int placement) {
        auto levelCellMain = this->m_mainLayer;
        if (!levelCellMain) return;

        if (auto gdlLabel = static_cast<CCLabelBMFont*>(levelCellMain->getChildByID("gdl-label"_spr))) {
            auto gdlIcon = levelCellMain->getChildByID("gdl-icon"_spr);

            if (placement == -1) gdlLabel->setString("N/A");
            else if (placement > 0) gdlLabel->setString(fmt::format("#{}", placement).c_str());
            else removePlacement(m_level->m_levelID, gdlLabel, gdlIcon, m_fields->m_origPositions, true);
        }
    }
};

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    static void onModify(auto& self) {
        if (!self.setHookPriorityAfterPost("LevelInfoLayer::init", "gdutilsdevs.gdutils")) {
            log::warn("Failed to set hook priority.");
        }
    }

    struct Fields {
        TaskHolder<web::WebResponse> m_listener;
        std::unordered_map<CCNode*, float> m_origPositions;
    };

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        if (!level || level->m_levelType == GJLevelType::Main || level->m_levelType == GJLevelType::Editor || g_levelsWithoutPlacement.contains(level->m_levelID.value())) return true;

        bool isDemon =
            level->m_demonDifficulty == static_cast<int>(DemonDifficultyType::ExtremeDemon) ||
            level->m_demonDifficulty == static_cast<int>(DemonDifficultyType::InsaneDemon);

        std::unordered_map<CCNode*, float>& origPositions = m_fields->m_origPositions;
        if (isDemon || level->m_stars == 0) {
            auto downloadIcon = this->getChildByID("downloads-icon");
            auto downloadLabel = this->m_downloadsLabel;
            auto likesIcon = this->m_likesIcon;
            auto likesLabel = this->m_likesLabel;
            auto lengthIcon = this->getChildByID("length-icon");
            auto lengthLabel = this->m_lengthLabel;
            auto exactLengthLabel = this->getChildByID("exact-length-label");
            auto orbIcon = this->m_orbsIcon;
            auto orbLabel = this->m_orbsLabel;

            origPositions[downloadIcon] = downloadIcon->getPositionY();
            origPositions[downloadLabel] = downloadLabel->getPositionY();
            origPositions[likesIcon] = likesIcon->getPositionY();
            origPositions[likesLabel] = likesLabel->getPositionY();
            origPositions[lengthIcon] = lengthIcon->getPositionY();
            origPositions[lengthLabel] = lengthLabel->getPositionY();
            origPositions[exactLengthLabel] = exactLengthLabel->getPositionY();
            origPositions[orbIcon] = orbIcon->getPositionY();
            origPositions[orbLabel] = orbLabel->getPositionY();
            downloadIcon->setPositionY(downloadIcon->getPositionY() + 14.0f);
            downloadLabel->setPositionY(downloadLabel->getPositionY() + 14.0f);
            likesIcon->setPositionY(likesIcon->getPositionY() + 14.0f);
            likesLabel->setPositionY(likesLabel->getPositionY() + 14.0f);
            lengthIcon->setPositionY(lengthIcon->getPositionY() + 14.0f);
            lengthLabel->setPositionY(lengthLabel->getPositionY() + 14.0f);
            if (exactLengthLabel) exactLengthLabel->setPositionY(exactLengthLabel->getPositionY() + 14.0f);
            orbIcon->setPositionY(orbIcon->getPositionY() + 14.0f);
            orbLabel->setPositionY(orbLabel->getPositionY() + 14.0f);

            float gdlIconY = 0.0f;
            if (orbIcon->isVisible()) gdlIconY = orbIcon->getPositionY() - (lengthIcon->getPositionY() - orbIcon->getPositionY());
            else gdlIconY = lengthIcon->getPositionY() - (likesIcon->getPositionY() - lengthIcon->getPositionY());

            float gdlIconX = lengthIcon->getPositionX() + lengthIcon->getContentWidth() / 2.0f;

            auto gdlIcon = CCSprite::create("global-list.png"_spr);
            gdlIcon->setContentSize({ 23.5f, 23.5f });
            gdlIcon->setScale(0.85f);
            gdlIcon->setAnchorPoint({ 0.5f, 0.5f });
            gdlIcon->setPosition({ gdlIconX, gdlIconY });
            gdlIcon->refreshTextureRect();
            gdlIcon->setID("gdl-icon"_spr);
            this->addChild(gdlIcon);

            auto gdlLabel = CCLabelBMFont::create("...", "bigFont.fnt");
            gdlLabel->setScale(0.5f);
            gdlLabel->setAnchorPoint({ 0.0f, 0.5f });
            gdlLabel->setPosition({ lengthLabel->getPositionX(), gdlIconY });
            gdlLabel->setID("gdl-label"_spr);
            this->addChild(gdlLabel);

            getPlacement(level->m_levelID.value());
        }

        return true;
    }

    void getPlacement(int levelID) {
        if (g_placementsCache.contains(levelID)) {
            int placement = g_placementsCache[levelID];
            updatePlacement(placement);
        }
        else {
            m_fields->m_listener.spawn(web::WebRequest().get(fmt::format("https://api.demonlist.org/level/classic/get?ingame_id={}", levelID)),
            [this, levelID](web::WebResponse value) {
                int placement = 0;
                if (!value.ok()) {
                    if (value.code() == -1) placement = -1;
                    else if (value.code() == 404) placement = 0;
                }
                else {
                    auto data = value.json();
                    matjson::Value json = data.ok().value();

                    const matjson::Value levelData = json["data"];

                    placement = levelData["placement"].asInt().ok().value();

                    if (placement > 0) {
                        g_placementsCache[levelID] = placement;
                    }
                    else placement = -1;
                }

                updatePlacement(placement);
            });
        }
    }

    void updatePlacement(int placement) {
        if (auto gdlLabel = static_cast<CCLabelBMFont*>(this->getChildByID("gdl-label"_spr))) {
            auto gdlIcon = this->getChildByID("gdl-icon"_spr);

            if (placement == -1) gdlLabel->setString("N/A");
            else if (placement > 0) gdlLabel->setString(fmt::format("#{}", placement).c_str());
            else removePlacement(m_level->m_levelID, gdlLabel, gdlIcon, m_fields->m_origPositions, false);
        }
    }
};

class $modify(MyLevelSearchLayer, LevelSearchLayer) {
    bool init(int type) {
        if (!LevelSearchLayer::init(type)) return false;

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