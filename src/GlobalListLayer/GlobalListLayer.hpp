#include <Geode/Geode.hpp>
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/binding/SetIDPopupDelegate.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>

class GlobalListLevel {
public:
	int levelID;
	std::string name;
	int place;

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
	void keyDown(cocos2d::enumKeyCodes) override;
	void keyBackClicked() override;

	~GlobalListLayer() override;
protected:
	geode::EventListener<geode::utils::web::WebTask> m_listener;
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
	int m_page = 0;
	int m_lvlsPerPage = 10;
	std::string m_query;
	std::string m_searchBarText;
	std::vector<std::string> m_searchResults;



	bool init() override;
	void failure(int code);
	void populateList(const std::string& query);
	void search();
	void page(int page);
	void setupPageInfo(gd::string, const char*) override;
	void showLoading();
	void loadLevelsFinished(cocos2d::CCArray* levels, const char* key, int) override;
	void setIDPopupClosed(SetIDPopup*, int) override;
};