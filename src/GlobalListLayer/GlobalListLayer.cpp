#include "./GlobalListLayer.hpp"
#include <Geode/binding/AppDelegate.hpp>
#include <Geode/binding/CustomListView.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/InfoAlertButton.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/SetIDPopup.hpp>
#include <Geode/loader/Mod.hpp>
#include <random>

using namespace geode::prelude;

#define API_URL "https://api.demonlist.org/levels/classic"

GlobalListLayer* GlobalListLayer::create() {
	auto ret = new GlobalListLayer();
	if (ret->init()) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

CCScene* GlobalListLayer::scene() {
	auto ret = CCScene::create();
	AppDelegate::get()->m_runningScene = ret;
	ret->addChild(GlobalListLayer::create());
	return ret;
}

constexpr const char* globalListInfo =
"A Demonlist of <cr>all Extreme Demons</c> worth rating.\nBased on <cy>demonlist.org.</c>";

// This layer contains hiimjasmine00's work (Integrated DemonList).
// I'm too silly to do something like this completely on my own =3
bool GlobalListLayer::init() {
	if (!CCLayer::init()) return false;
	setID("GlobalListLayer");

	auto winSize = CCDirector::get()->getWinSize();

	auto GlobalListBG = CCSprite::create("global-list-bg.png"_spr);
	GlobalListBG->setAnchorPoint({ 0.5f, 0.5f });
	GlobalListBG->setScale(winSize.height / GlobalListBG->getTextureRect().size.height);
	if (winSize.width > GlobalListBG->getContentWidth() * GlobalListBG->getScale())
		GlobalListBG->setScale(winSize.width / GlobalListBG->getTextureRect().size.width);
	GlobalListBG->setPosition({ winSize.width / 2, winSize.height / 2 });
	GlobalListBG->setZOrder(0);
	GlobalListBG->setID("global-list-backgrownd");
	addChild(GlobalListBG);

	m_levelsCountLabel = CCLabelBMFont::create("", "goldFont.fnt");
	m_levelsCountLabel->setAnchorPoint({ 1.0f, 1.0f });
	m_levelsCountLabel->setScale(0.45f);
	m_levelsCountLabel->setPosition({ winSize.width - 7.0f, winSize.height - 3.0f });
	m_levelsCountLabel->setID("level-count-label");
	addChild(m_levelsCountLabel);

	m_levelList = GJListLayer::create(
		nullptr,
		"Global DemonList",
		{ 0, 0, 0, 180 },
		356.0f, 220.0f, 0);
	m_levelList->setPosition(winSize / 2.0f - m_levelList->getContentSize() / 2.0f);
	m_levelList->setColor({ 194, 114, 62 });
	m_levelList->setID("GJListLayer");
	addChild(m_levelList, 5);

	m_searchBarMenu = CCMenu::create();
	m_searchBarMenu->setContentSize({ 356.0f, 30.0f });
	m_searchBarMenu->setPosition({ 0.0f, 190.0f });
	m_searchBarMenu->setID("search-bar-menu");
	m_levelList->addChild(m_searchBarMenu);

	auto searchBarBG = CCLayerColor::create({ 194, 114, 62, 255 }, 356.0f, 30.0f);
	searchBarBG->setID("search-bar-backgrownd");
	m_searchBarMenu->addChild(searchBarBG);

	auto searchBtn = CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto) { search(); });
	searchBtn->setPosition({ m_searchBarMenu->getContentWidth() -
							 searchBtn->getContentWidth() * searchBtn->getScaleX() / 2.0f - 15.0f, 15.0f});
	searchBtn->setID("search-button");
	m_searchBarMenu->addChild(searchBtn);

	m_searchBar = TextInput::create(400.0f, "Search levels...");
	m_searchBar->setMaxCharCount(35);
	m_searchBar->setPosition({ 165.0f, 15.0f });
	m_searchBar->setTextAlign(TextInputAlign::Left);
	m_searchBar->getInputNode()->setLabelPlaceholderScale(0.70f);
	m_searchBar->getInputNode()->setMaxLabelScale(0.70f);
	m_searchBar->setScale(0.75f);
	m_searchBar->setID("search-bar");
	m_searchBarMenu->addChild(m_searchBar);

	auto btnsMenu = CCMenu::create();
	btnsMenu->setPosition({ 0.0f, 0.0f });
	btnsMenu->setID("buttons-menu");
	addChild(btnsMenu, 2);

	m_backButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
		CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
	});
	m_backButton->setPosition({ 25.0f, winSize.height - 25.0f });
	m_backButton->setID("back-button");
	btnsMenu->addChild(m_backButton);

	m_leftButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 1.0f, [this](auto) { page(m_page - 1); });
	m_leftButton->setPosition({ 24.0f, winSize.height / 2.0f });
	m_leftButton->setID("prev-page-button");
	btnsMenu->addChild(m_leftButton);

	auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	rightBtnSpr->setFlipX(true);
	m_rightButton = CCMenuItemExt::createSpriteExtra(rightBtnSpr, [this](auto) { page(m_page + 1); });
	m_rightButton->setPosition({ winSize.width - 24.0f, winSize.height / 2.0f });
	m_rightButton->setID("next-page-button");
	btnsMenu->addChild(m_rightButton);

	m_infoButton = InfoAlertButton::create("Global DemonList", globalListInfo, 1.0f);
	m_infoButton->setPosition({ 30.0f, 30.0f });
	m_infoButton->setID("info-button");
	btnsMenu->addChild(m_infoButton);

	auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
	auto refreshButton = CCMenuItemExt::createSpriteExtra(refreshBtnSpr, [this](auto) {
		showLoading();
		loadGlobalList();
	});
	refreshButton->setPosition({ winSize.width - refreshBtnSpr->getContentWidth() / 2.0f - 4.0f, refreshBtnSpr->getContentHeight() / 2.0f + 4.0f });
	refreshButton->setID("refresh-button");
	btnsMenu->addChild(refreshButton);

	auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
	pageBtnSpr->setScale(0.7f);
	m_pageLabel = CCLabelBMFont::create("1", "bigFont.fnt");
	m_pageLabel->setScale(0.8f);
	m_pageLabel->setPosition(pageBtnSpr->getContentSize() / 2.0f);
	pageBtnSpr->addChild(m_pageLabel);
	m_pageButton = CCMenuItemExt::createSpriteExtra(pageBtnSpr, [this](auto) {
		auto popup = SetIDPopup::create(m_page + 1, 1, (m_searchResults.size() + (m_lvlsPerPage == 10 ? 9 : 19)) / m_lvlsPerPage, "Go to Page", "Go", true, 1, 60.0f, false, false);
		popup->m_delegate = this;
		popup->show();
	});
	m_pageButton->setPosition({ winSize.width - m_pageButton->getContentWidth() * m_pageButton->getScaleX() / 2.0f - 3.0f, winSize.height - 39.5f });
	m_pageButton->setID("page-button");
	btnsMenu->addChild(m_pageButton);

	m_randomButton = CCMenuItemExt::createSpriteExtraWithFilename("BI_randomBtn_001.png"_spr, 0.9f, [this](auto) {
		static std::mt19937 mt(std::random_device{}());
		page(std::uniform_int_distribution<int>(0, (m_searchResults.size() - 1) / m_lvlsPerPage)(mt));
	});
	m_randomButton->setPositionY(
		m_pageButton->getPositionY() - m_pageButton->getContentHeight() / 2.0f - m_randomButton->getContentHeight() / 2.0f - 5.0f);
	m_randomButton->setID("random-button");
	btnsMenu->addChild(m_randomButton);

	auto lastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	lastArrow->setFlipX(true);
	auto otherLastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	otherLastArrow->setPosition(lastArrow->getContentSize() / 2.0f + CCPoint{ 20.0f, 0.0f });
	otherLastArrow->setFlipX(true);
	lastArrow->addChild(otherLastArrow);
	lastArrow->setScale(0.4f);
	m_lastButton = CCMenuItemExt::createSpriteExtra(lastArrow, [this](auto) {
		page((m_searchResults.size() - 1) / m_lvlsPerPage);
		});
	m_lastButton->setPositionY(
		m_randomButton->getPositionY() - m_randomButton->getContentHeight() / 2.0f - m_lastButton->getContentHeight() / 2.0f - 5.0f);
	m_lastButton->setID("last-button");
	btnsMenu->addChild(m_lastButton);

	auto x = winSize.width - m_randomButton->getContentWidth() / 2.0f - 3.0f;
	m_pageButton->setPositionX(x);
	m_randomButton->setPositionX(x);
	m_lastButton->setPositionX(x - 4.0f);

	auto firstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	auto otherFirstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	otherFirstArrow->setPosition(firstArrow->getContentSize() / 2.0f - CCPoint{ 20.0f, 0.0f });
	firstArrow->addChild(otherFirstArrow);
	firstArrow->setScale(0.4f);
	m_firstButton = CCMenuItemExt::createSpriteExtra(firstArrow, [this](auto) {
		page(0);
		});
	m_firstButton->setPosition({ 21.5f, m_lastButton->getPositionY() });
	m_firstButton->setID("first-button");
	btnsMenu->addChild(m_firstButton);

	m_loadingCircle = LoadingCircle::create();
	m_loadingCircle->setParentLayer(this);
	m_loadingCircle->setID("loading-circle");
	m_loadingCircle->show();

	setKeypadEnabled(true);
	setKeyboardEnabled(true);
	showLoading();

	loadGlobalList();

	return true;
}

void GlobalListLayer::loadGlobalList() {
	std::string url = "https://api.demonlist.org/levels/classic";
	auto req = web::WebRequest();

	m_listener.bind([this](web::WebTask::Event* e) {
		if (auto res = e->getValue()) {
			if (!res->ok()) {
				log::error("Failed to load GlobalList. Failed code: {}", res->code());
				failure(res->code());
				return;
			}
			m_levels.clear();
			auto data = res->json();

			matjson::Value json = data.unwrapOr(matjson::Value::array());
			if (!json.contains("data") || !json["data"].isArray() || json["data"].size() == 0) {
				failure(204);
				return;
			}

			for (const auto& level : json["data"]) {
				int levelID = level["level_id"].asInt().unwrapOr(0);
				std::string name = level["name"].asString().unwrapOr("");
				int place = level["place"].asInt().unwrapOr(0);

				if (levelID == 0 || name == "" || place == 0) {
					failure(204);
					return;
				}

				m_levels.push_back({ levelID, name, place });
			}

			populateList(m_query);
		}
	});
	auto task = req.get(url);
	m_listener.setFilter(task);
}

void GlobalListLayer::populateList(const std::string& query) {
	auto winSize = CCDirector::get()->getWinSize();
	m_searchResults.clear();

	if (query.empty()) {
		for (auto& level : m_levels) {
			m_searchResults.push_back(std::to_string(level.levelID));
		}
	}
	else {
		auto lowerQuery = string::toLower(query);
		for (auto& level : m_levels) {
			if (string::toLower(level.name).find(lowerQuery) == std::string::npos &&
				std::to_string(level.levelID) != lowerQuery) continue;
			m_searchResults.push_back(std::to_string(level.levelID));
		}
	}

	m_query = query;

	if (m_searchResults.empty()) {
		loadLevelsFinished(CCArray::create(), "", 0);
	}
	else {
		auto glm = GameLevelManager::get();
		glm->m_levelManagerDelegate = this;

		auto searchObject = GJSearchObject::create(SearchType::Type26);

		std::string searchQuery;
		auto gm = GameManager::sharedState();
		if (gm->getGameVariable("0093")) {
			m_lvlsPerPage = 20;
		}

		for (int i = m_lvlsPerPage * m_page; i < std::min(m_lvlsPerPage * (m_page + 1), int(m_searchResults.size())); i++) {
			if (!searchQuery.empty()) searchQuery += ",";
			searchQuery += m_searchResults[i];
		}
		searchObject->m_searchQuery = searchQuery;

		std::string_view key = searchObject->getKey();
		if (auto storedLevels = glm->getStoredOnlineLevels(key.substr(std::max<ptrdiff_t>(0, key.size() - 256)).data())) {
			loadLevelsFinished(storedLevels, "", 0);
			setupPageInfo("", "");
		}
		else {
			glm->getOnlineLevels(searchObject);
		}
	}
}

void GlobalListLayer::search() {
	auto query = m_searchBar->getString();
	if (m_query != query) {
		showLoading();

		m_page = 0;
		populateList(query);
	}
}

void GlobalListLayer::loadLevelsFinished(CCArray* levels, const char*, int) {
	if (auto listView = m_levelList->m_listView) {
		listView->removeFromParent();
		listView->release();
	}

	auto listView = CustomListView::create(levels, BoomListType::Level, 190.0f, 356.0f);
	listView->retain();
	m_levelList->addChild(listView, 6, 9);
	m_levelList->m_listView = listView;

	m_searchBarMenu->setVisible(true);
	m_levelsCountLabel->setVisible(true);
	m_loadingCircle->setVisible(false);
	auto size = m_searchResults.size();
	if (size > m_lvlsPerPage) {
		auto maxPage = (size - 1) / m_lvlsPerPage;
		m_leftButton->setVisible(m_page > 0);
		m_rightButton->setVisible(m_page < maxPage);
		m_firstButton->setVisible(m_page > 0);
		m_lastButton->setVisible(m_page < maxPage);
		m_pageButton->setVisible(true);
		m_randomButton->setVisible(true);
	}
	if (m_page + 1 > 99) m_pageLabel->setScale(0.5f);
	else m_pageLabel->setScale(0.8f);
}

void GlobalListLayer::page(int page) {
	int maxPage = (m_searchResults.size() + (m_lvlsPerPage == 10 ? 9 : 19)) / m_lvlsPerPage;
	m_page = page <= maxPage ? (page < 0 ? 0 : page) : maxPage;

	showLoading();
	populateList(m_query);
}

void GlobalListLayer::setupPageInfo(gd::string, const char*) {
	m_levelsCountLabel->setString(fmt::format("{} to {} of {}", m_page * m_lvlsPerPage + 1,
		std::min<int>(m_searchResults.size(), (m_page + 1) * m_lvlsPerPage), m_searchResults.size()).c_str());
	m_levelsCountLabel->limitLabelWidth(100.0f, 0.6f, 0.0f);
}

void GlobalListLayer::showLoading() {
	m_pageLabel->setString(fmt::to_string(m_page + 1).c_str());
	m_loadingCircle->setVisible(true);
	if (auto listView = m_levelList->m_listView) listView->setVisible(false);
	m_searchBarMenu->setVisible(false);
	m_levelsCountLabel->setVisible(false);
	m_leftButton->setVisible(false);
	m_rightButton->setVisible(false);
	m_firstButton->setVisible(false);
	m_lastButton->setVisible(false);
	m_pageButton->setVisible(false);
	m_randomButton->setVisible(false);
}

void GlobalListLayer::failure(int code) {
	auto alertLayer = FLAlertLayer::create(
		fmt::format("Load failed ({})", code).c_str(),
		"Failed to load GlobalList. Please try again.",
		"OK"
	);
	alertLayer->m_scene = this;
	alertLayer->show();

	m_loadingCircle->setVisible(false);
}

void GlobalListLayer::setIDPopupClosed(SetIDPopup*, int page) {
	m_page = std::clamp<int>(page - 1, 0, (m_searchResults.size() - 1) / m_lvlsPerPage);
	showLoading();
	populateList(m_query);
}

void GlobalListLayer::keyDown(enumKeyCodes key) {
	switch (key) {
	case KEY_Left:
	case CONTROLLER_Left:
		if (m_leftButton->isVisible()) page(m_page - 1);
		break;
	case KEY_Right:
	case CONTROLLER_Right:
		if (m_rightButton->isVisible()) page(m_page + 1);
		break;
	case KEY_Enter:
		search();
		break;
	default:
		CCLayer::keyDown(key);
		break;
	}
}

void GlobalListLayer::keyBackClicked() {
	CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}

GlobalListLayer::~GlobalListLayer() {
	auto glm = GameLevelManager::get();
	if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
}