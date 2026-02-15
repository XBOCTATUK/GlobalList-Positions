#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/binding/SetIDPopupDelegate.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include "../../include/tsl/ordered_map.h"

using namespace geode::prelude;

struct LevelFilters {
	std::vector<bool> lengthFilter = { false, false, false, false, false }; // Short, Medium, Long, XL, Custom
	std::vector<bool> diffFilter = { false, false, false, false, false }; // Top 50, Top 150, Top 300, Beyond, Custom
	std::vector<int> customLengthFilter = { 0, 0 }; // begin, end
	std::vector<int> customDiffFilter = { 0, 0 }; // begin, end
	bool rated = false;
	bool unrated = false;
	bool completed = false;
	bool byHolder = false;
	std::string username = "";
	std::string holder = "";

	bool isDefault() {
		return
			!this->lengthFilter[0] && !this->lengthFilter[1] && !this->lengthFilter[2] && !this->lengthFilter[3] && !this->lengthFilter[4] && !this->lengthFilter[5] &&
			!this->diffFilter[0] && !this->diffFilter[1] && !this->diffFilter[2] && !this->diffFilter[3] && !this->diffFilter[4] &&
			!this->rated && !this->unrated && !this->completed && !this->byHolder;
	}

	bool isDataRequired() {
		return this->rated || this->unrated || this->completed || this->byHolder;
	}
};

struct LevelData {
	bool rated;
	bool unrated;
	gd::string holder;
};

class GlobalListLevel {
public:
	int id;
	int levelID;
	std::string name;
	int placement;
	int length;

	bool contains(std::string obj) {
		if (this->name.find(obj) != std::string::npos) return true;
		else if (std::to_string(this->levelID) == obj) return true;
		else return false;
	}
};

class GlobalListLayer : public cocos2d::CCLayer, SetIDPopupDelegate, LevelManagerDelegate {
public:
	std::vector<GlobalListLevel> m_levels;

	static GlobalListLayer* create();
	static cocos2d::CCScene* scene();

	void loadGlobalList();
	void keyDown(cocos2d::enumKeyCodes, double d) override;
	void keyBackClicked() override;

	~GlobalListLayer() override;
	friend class FilterPopup;
	friend class LoadingPopup;

protected:
	TaskHolder<web::WebResponse> m_listener;
	GJListLayer* m_levelList;
	cocos2d::CCMenu* m_searchBarMenu;
	geode::TextInput* m_searchBar;
	cocos2d::CCLabelBMFont* m_levelsCountLabel;
	cocos2d::CCLabelBMFont* m_pageLabel;
	CCMenuItemSpriteExtra* m_pageButton;
	CCMenuItemSpriteExtra* m_backButton;
	CCMenuItemSpriteExtra* m_leftButton;
	CCMenuItemSpriteExtra* m_rightButton;
	InfoAlertButton* m_infoButton;
	CCMenuItemSpriteExtra* m_randomButton;
	CCMenuItemSpriteExtra* m_firstButton;
	CCMenuItemSpriteExtra* m_lastButton;
	LoadingCircle* m_loadingCircle;
	size_t m_page = 0;
	size_t m_lvlsPerPage = 10;
	std::string m_query;
	std::vector<int> m_searchResults;

	bool init() override;
	void failure(int code);
	void populateList(const std::string& query, bool useFilters = true);
	void search();
	void page(size_t page);
	void setupPageInfo(gd::string, const char*) override;
	void showLoading();
	bool isSuitable(GlobalListLevel level);
	void loadLevelsFinished(cocos2d::CCArray* levels, const char* key, int) override;
	virtual void loadLevelsFailed(char const* key) override;
	void setIDPopupClosed(SetIDPopup*, int) override;
};

class FilterPopup : public geode::Popup, LevelManagerDelegate {
public:
	static FilterPopup* create();
	friend class LoadingPopup;

	CCMenuItemToggler* m_ratedFilterToggler;
	CCMenuItemToggler* m_unratedFilterToggler;

protected:
	bool init();
	void changeLengthFilter(int lengthType, CCLabelBMFont* label);
	void changeDiffFilter(int diffType, CCLabelBMFont* label);
};

class RangePopup : public geode::Popup {
public:
	static RangePopup* create(int filterType); // 0 - Length, 1 - Difficulty

protected:
	int m_filterType = 0;
	TextInput* m_fromTextInput;
	TextInput* m_toTextInput;

	bool init(int filterType);
	virtual void onClose(cocos2d::CCObject*) override;
};

class LoadingPopup : public geode::Popup, LevelManagerDelegate {
public:
	static LoadingPopup* create();

protected:
	TaskHolder<web::WebResponse> m_listener;
	std::vector<int> m_requiredLevels;
	bool m_loading = false;
	bool m_playerInfoLoading = false;
	int m_currentBatch = 0;

	Slider* m_loadingProgress;
	CCLabelBMFont* m_loadingLabel;

	bool init();
	void getLevels();
	void loadBatch();
	void afterLoading();
	void loadPlayerInfo(std::string username);
	void loadPlayerRecords(int id, std::string username);
	virtual void loadLevelsFinished(cocos2d::CCArray* levels, char const* key) override;
	virtual void loadLevelsFailed(char const* key) override;
	void failure(int code);
	virtual void keyBackClicked() override;
	virtual void onClose(cocos2d::CCObject*) override;
};

inline std::unordered_map<int, int> g_placementsCache;
inline std::unordered_map<int, bool> g_levelsWithoutPlacement;
inline std::unordered_map<int, LevelData> g_levelsData;
inline std::unordered_map<std::string, std::vector<int>> g_usersRecords;
inline tsl::ordered_map<int, GlobalListLevel> g_levels;
inline LevelFilters g_levelFilters;
inline LevelFilters g_storedFilters;
inline const LevelFilters g_defaultFilters; 