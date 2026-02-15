#include "./GlobalListLayer.hpp"

FilterPopup* FilterPopup::create() {
	auto ret = new FilterPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

constexpr const char* filterWarning =
"Many features in this filter request data from <cy>RobTop servers</c>. The game's servers automatically set usage limits."
"To avoid a temporary ban(up to 1 hour), please avoid updating too frequently."
"Click <cr>I confirm</c> to continue, confirming your understanding of this limitation.";

bool FilterPopup::init() {
	if (!Popup::init(450.0f, 280.0f)) return false;

	this->setID("filter-menu");
	//this->setZOrder(100);
	this->setTitle("Search filters");

	auto lengthFilterBG = CCScale9Sprite::create("square02b_001.png");
	lengthFilterBG->setColor({ 123, 68, 41 });
	lengthFilterBG->setContentSize({ 365.0f, 35.0f });
	lengthFilterBG->setPosition({ 225.0f, 225.0f });
	m_mainLayer->addChild(lengthFilterBG);

	auto lengthFilterMenu = CCMenu::create();
	lengthFilterMenu->setLayout(RowLayout::create()
		->setGap(16.0f)
		->setAxisAlignment(AxisAlignment::Between)
		->setAutoScale(false));
	lengthFilterMenu->setContentSize({ 340.0f, 35.0f });
	lengthFilterMenu->setPosition({ 225.0f, 225.0f });
	lengthFilterMenu->ignoreAnchorPointForPosition(false);
	m_mainLayer->addChild(lengthFilterMenu);

	auto clockSpr = CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png");
	clockSpr->setContentSize({ 23.0f, 23.0f });
	lengthFilterMenu->addChild(clockSpr);

	auto shortLenLabel = CCLabelBMFont::create("Short", "bigFont.fnt");
	shortLenLabel->setScale(0.42f);
	if (g_levelFilters.lengthFilter[0]) shortLenLabel->setColor({ 255, 255, 255 });
	else shortLenLabel->setColor({ 125, 125, 125 });
	auto shortLenBtn = CCMenuItemExt::createSpriteExtra(shortLenLabel, ([this, shortLenLabel](auto) {
		changeLengthFilter(0, shortLenLabel);
		}));
	lengthFilterMenu->addChild(shortLenBtn);

	auto mediumLenLabel = CCLabelBMFont::create("Medium", "bigFont.fnt");
	mediumLenLabel->setScale(0.42f);
	if (g_levelFilters.lengthFilter[1]) mediumLenLabel->setColor({ 255, 255, 255 });
	else mediumLenLabel->setColor({ 125, 125, 125 });
	auto mediumLenBtn = CCMenuItemExt::createSpriteExtra(mediumLenLabel, [this, mediumLenLabel](auto) {
		changeLengthFilter(1, mediumLenLabel);
		});
	lengthFilterMenu->addChild(mediumLenBtn);

	auto longLenLabel = CCLabelBMFont::create("Long", "bigFont.fnt");
	longLenLabel->setScale(0.42f);
	if (g_levelFilters.lengthFilter[2]) longLenLabel->setColor({ 255, 255, 255 });
	else longLenLabel->setColor({ 125, 125, 125 });
	auto longLenBtn = CCMenuItemExt::createSpriteExtra(longLenLabel, [this, longLenLabel](auto) {
		changeLengthFilter(2, longLenLabel);
		});
	lengthFilterMenu->addChild(longLenBtn);

	auto xlLenLabel = CCLabelBMFont::create("XL", "bigFont.fnt");
	xlLenLabel->setScale(0.42f);
	if (g_levelFilters.lengthFilter[3]) xlLenLabel->setColor({ 255, 255, 255 });
	else xlLenLabel->setColor({ 125, 125, 125 });
	auto xlLenBtn = CCMenuItemExt::createSpriteExtra(xlLenLabel, [this, xlLenLabel](auto) {
		changeLengthFilter(3, xlLenLabel);
		});
	lengthFilterMenu->addChild(xlLenBtn);

	auto customLenLabel = CCLabelBMFont::create("Custom", "bigFont.fnt");
	customLenLabel->setScale(0.42f);
	if (g_levelFilters.lengthFilter[4]) customLenLabel->setColor({ 255, 255, 255 });
	else customLenLabel->setColor({ 125, 125, 125 });
	auto customLenBtn = CCMenuItemExt::createSpriteExtra(customLenLabel, [this, customLenLabel](auto) {
		changeLengthFilter(4, customLenLabel);
		});
	lengthFilterMenu->addChild(customLenBtn);

	auto optionsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
	optionsSpr->setContentSize({ 50.5f, 50.5f });
	optionsSpr->setScale(0.46);
	auto customLenOptionsBtn = CCMenuItemExt::createSpriteExtra(optionsSpr, [this](auto) { RangePopup::create(0)->show(); });
	customLenOptionsBtn->updateLayout();
	lengthFilterMenu->addChild(customLenOptionsBtn);

	lengthFilterMenu->updateLayout();


	auto diffFilterBG = CCScale9Sprite::create("square02b_001.png");
	diffFilterBG->setColor({ 123, 68, 41 });
	diffFilterBG->setContentSize({ 365.0f, 35.0f });
	diffFilterBG->setPosition({ 225.0f, 180.0f });
	m_mainLayer->addChild(diffFilterBG);

	auto diffFilterMenu = CCMenu::create();
	diffFilterMenu->setLayout(RowLayout::create()
		->setGap(16.0f)
		->setAxisAlignment(AxisAlignment::Between)
		->setAutoScale(false));
	diffFilterMenu->setContentSize({ 340.0f, 35.0f });
	diffFilterMenu->setPosition({ 225.0f, 180.0f });
	diffFilterMenu->ignoreAnchorPointForPosition(false);
	m_mainLayer->addChild(diffFilterMenu);

	auto demonSpr = CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");
	demonSpr->setContentSize({ 26.5f, 26.5f });
	demonSpr->setScale(0.90f);
	diffFilterMenu->addChild(demonSpr);

	auto top50Label = CCLabelBMFont::create("Top 50", "bigFont.fnt");
	top50Label->setScale(0.36f);
	if (g_levelFilters.diffFilter[0]) top50Label->setColor({ 255, 255, 255 });
	else top50Label->setColor({ 125, 125, 125 });
	auto top50Btn = CCMenuItemExt::createSpriteExtra(top50Label, [this, top50Label](auto) {
		changeDiffFilter(0, top50Label);
		});
	diffFilterMenu->addChild(top50Btn);

	auto top150Label = CCLabelBMFont::create("Top 150", "bigFont.fnt");
	top150Label->setScale(0.36f);
	if (g_levelFilters.diffFilter[1]) top150Label->setColor({ 255, 255, 255 });
	else top150Label->setColor({ 125, 125, 125 });
	auto top150Btn = CCMenuItemExt::createSpriteExtra(top150Label, [this, top150Label](auto) {
		changeDiffFilter(1, top150Label);
		});
	diffFilterMenu->addChild(top150Btn);

	auto top300Label = CCLabelBMFont::create("Top 300", "bigFont.fnt");
	top300Label->setScale(0.36f);
	if (g_levelFilters.diffFilter[2]) top300Label->setColor({ 255, 255, 255 });
	else top300Label->setColor({ 125, 125, 125 });
	auto top300Btn = CCMenuItemExt::createSpriteExtra(top300Label, [this, top300Label](auto) {
		changeDiffFilter(2, top300Label);
		});
	diffFilterMenu->addChild(top300Btn);

	auto beyondLabel = CCLabelBMFont::create("Beyond", "bigFont.fnt");
	beyondLabel->setScale(0.36f);
	if (g_levelFilters.diffFilter[3]) beyondLabel->setColor({ 255, 255, 255 });
	else beyondLabel->setColor({ 125, 125, 125 });
	auto beyondBtn = CCMenuItemExt::createSpriteExtra(beyondLabel, [this, beyondLabel](auto) {
		changeDiffFilter(3, beyondLabel);
		});
	diffFilterMenu->addChild(beyondBtn);

	auto customDiffLabel = CCLabelBMFont::create("Custom", "bigFont.fnt");
	customDiffLabel->setScale(0.36f);
	if (g_levelFilters.diffFilter[4]) customDiffLabel->setColor({ 255, 255, 255 });
	else customDiffLabel->setColor({ 125, 125, 125 });
	auto customDiffBtn = CCMenuItemExt::createSpriteExtra(customDiffLabel, [this, customDiffLabel](auto) {
		changeDiffFilter(4, customDiffLabel);
		});
	diffFilterMenu->addChild(customDiffBtn);

	auto customDiffOptionsBtn = CCMenuItemExt::createSpriteExtra(optionsSpr, [this](auto) { RangePopup::create(1)->show(); });
	customDiffOptionsBtn->updateLayout();
	diffFilterMenu->addChild(customDiffOptionsBtn);

	diffFilterMenu->updateLayout();


	auto otherFiltersBG = CCScale9Sprite::create("square02b_001.png");
	otherFiltersBG->setColor({ 123, 68, 41 });
	otherFiltersBG->setContentSize({ 365.0f, 100.0f });
	otherFiltersBG->setPosition({ 225.0f, 102.5f });
	m_mainLayer->addChild(otherFiltersBG);

	auto otherFiltersMenu = CCMenu::create();
	otherFiltersMenu->setContentSize({ 365.0f, 100.0f });
	otherFiltersMenu->setPosition({ 225.0f, 102.5f });
	otherFiltersMenu->ignoreAnchorPointForPosition(false);
	m_mainLayer->addChild(otherFiltersMenu);

	m_ratedFilterToggler = CCMenuItemExt::createTogglerWithStandardSprites(1.0f, [this](CCMenuItemToggler* toggler) {
		if (!toggler->isOn()) {
			g_levelFilters.rated = true;
			g_levelFilters.unrated = false;
			m_unratedFilterToggler->toggle(false);
		}
		else g_levelFilters.rated = false;
	});
	m_ratedFilterToggler->toggle(g_levelFilters.rated);
	m_ratedFilterToggler->setScale(0.75f);
	m_ratedFilterToggler->setPosition({ 19.0f, 81.0f });
	otherFiltersMenu->addChild(m_ratedFilterToggler);

	auto ratedFilterLabel = CCLabelBMFont::create("Rated", "bigFont.fnt");
	ratedFilterLabel->setAnchorPoint({ 0.0f, 0.5f });
	ratedFilterLabel->setScale(0.4f);
	ratedFilterLabel->setPosition({ 36.0f, 81.0f });
	otherFiltersMenu->addChild(ratedFilterLabel);

	m_unratedFilterToggler = CCMenuItemExt::createTogglerWithStandardSprites(1.0f, [this](CCMenuItemToggler* toggler) {
		if (!toggler->isOn()) {
			g_levelFilters.unrated = true;
			g_levelFilters.rated = false;
			m_ratedFilterToggler->toggle(false);
		}
		else g_levelFilters.unrated = false;
	});
	m_unratedFilterToggler->toggle(g_levelFilters.unrated);
	m_unratedFilterToggler->setScale(0.75f);
	m_unratedFilterToggler->setPosition({ 19.0f, 50.0f });
	otherFiltersMenu->addChild(m_unratedFilterToggler);

	auto unratedFilterLabel = CCLabelBMFont::create("Unrated", "bigFont.fnt");
	unratedFilterLabel->setAnchorPoint({ 0.0f, 0.5f });
	unratedFilterLabel->setScale(0.4f);
	unratedFilterLabel->setPosition({ 36.0f, 50.0f });
	otherFiltersMenu->addChild(unratedFilterLabel);

	auto completedFilterToggler = CCMenuItemExt::createTogglerWithStandardSprites(1.0f, [this](CCMenuItemToggler* toggler) {
		if (!toggler->isOn()) g_levelFilters.completed = true;
		else g_levelFilters.completed = false;
	});
	completedFilterToggler->toggle(g_levelFilters.completed);
	completedFilterToggler->setScale(0.75f);
	completedFilterToggler->setPosition({ 143.5f, 81.0f });
	otherFiltersMenu->addChild(completedFilterToggler);

	auto completedFilterLabel = CCLabelBMFont::create("Completed", "bigFont.fnt");
	completedFilterLabel->setAnchorPoint({ 0.0f, 0.5f });
	completedFilterLabel->setScale(0.4f);
	completedFilterLabel->setPosition({ 160.5f, 81.0f });
	otherFiltersMenu->addChild(completedFilterLabel);

	auto creatorFilterToggler = CCMenuItemExt::createTogglerWithStandardSprites(1.0f, [this](CCMenuItemToggler* toggler) {
		if (!toggler->isOn()) g_levelFilters.byHolder = true;
		else g_levelFilters.byHolder = false;
	});
	creatorFilterToggler->toggle(g_levelFilters.byHolder);
	creatorFilterToggler->setScale(0.75f);
	creatorFilterToggler->setPosition({ 143.5f, 50.0f });
	otherFiltersMenu->addChild(creatorFilterToggler);

	auto creatorFilterLabel = CCLabelBMFont::create("Creator", "bigFont.fnt");
	creatorFilterLabel->setAnchorPoint({ 0.0f, 0.5f });
	creatorFilterLabel->setScale(0.4f);
	creatorFilterLabel->setPosition({ 160.5f, 50.0f });
	otherFiltersMenu->addChild(creatorFilterLabel);

	auto usernameInput = TextInput::create(106.0f / 0.9f, "Username");
	usernameInput->setAnchorPoint({ 0.0f, 0.5f });
	usernameInput->setScale(0.9f);
	usernameInput->setPosition({ 254.0f, 81.0f });
	usernameInput->setMaxCharCount(32);
	usernameInput->setString(Mod::get()->getSavedValue<std::string>("username", ""));
	usernameInput->setCallback([this](const std::string& text) {
		Mod::get()->setSavedValue<std::string>("username", text);
	});
	otherFiltersMenu->addChild(usernameInput);

	auto holderInput = TextInput::create(106.0f / 0.9f, "Holder");
	holderInput->setAnchorPoint({ 0.0f, 0.5f });
	holderInput->setScale(0.9f);
	holderInput->setPosition({ 254.0f, 50.0f });
	holderInput->setMaxCharCount(32);
	holderInput->setString(Mod::get()->getSavedValue<std::string>("holder", ""));
	holderInput->setCallback([this](const std::string& text) {
		Mod::get()->setSavedValue<std::string>("holder", text);
	});
	otherFiltersMenu->addChild(holderInput);
	
	auto applyBtnSpr = ButtonSprite::create("Apply");
	auto applyBtn = CCMenuItemExt::createSpriteExtra(applyBtnSpr, [this, usernameInput, holderInput](auto) {
		if (!g_levelFilters.isDataRequired()) {
			g_storedFilters = g_levelFilters;
			auto scene = CCDirector::get()->getRunningScene();
			auto globalListLayer = static_cast<GlobalListLayer*>(scene->getChildByID("GlobalListLayer"));
			globalListLayer->showLoading();
			globalListLayer->populateList("");
			onClose(this);
			return;
		}

		auto time = std::chrono::high_resolution_clock::now();
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>( time.time_since_epoch() );
		int64_t timestamp = seconds.count();

		int lastUpdate = Mod::get()->getSavedValue<int64_t>("lastLevelDataUpdate", 0);
		int difference = timestamp - lastUpdate;
		log::info("{}", difference);
		if (lastUpdate != 0 && difference < 600) {
			FLAlertLayer::create(
				"Loading canceled",
				fmt::format("Please wait <cj>{}min {}sec</c> for the level information to load. If you want to use filters right now, please use only the length and difficulty filters.", (600 - difference) / 60, (600 - difference) % 60),
				"Ok"
			)->show();
			return;
		}

		g_levelFilters.username = usernameInput->getString();
		g_levelFilters.holder = holderInput->getString();
		
		if (!Mod::get()->getSavedValue<bool>("showWarning", true)) {
			LoadingPopup::create()->show();
			return;
		}

		createQuickPopup("Warning", filterWarning, "Oh, no", "I confirm", [](auto, bool confirmBtn) {
			if (confirmBtn) {
				Mod::get()->setSavedValue<bool>("showWarning", false);
				LoadingPopup::create()->show();
			}
		});
	});
	applyBtn->setPosition({ 225.0f, 28.0f });
	m_buttonMenu->addChild(applyBtn);

	return true;
}

void FilterPopup::changeLengthFilter(int lengthType, CCLabelBMFont* label) {
	bool lengthFilter = g_levelFilters.lengthFilter[lengthType];
	if (!lengthFilter) {
		label->setColor({ 255, 255, 255 });
		g_levelFilters.lengthFilter[lengthType] = true;
	}
	else {
		label->setColor({ 125, 125, 125 });
		g_levelFilters.lengthFilter[lengthType] = false;
	}
}

void FilterPopup::changeDiffFilter(int diffType, CCLabelBMFont* label) {
	bool lengthFilter = g_levelFilters.diffFilter[diffType];
	if (!lengthFilter) {
		label->setColor({ 255, 255, 255 });
		g_levelFilters.diffFilter[diffType] = true;
	}
	else {
		label->setColor({ 125, 125, 125 });
		g_levelFilters.diffFilter[diffType] = false;
	}
}