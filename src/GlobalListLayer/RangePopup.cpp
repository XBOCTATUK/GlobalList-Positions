#include "./GlobalListLayer.hpp"

RangePopup* RangePopup::create(int filterType) {
	auto ret = new RangePopup();
	if (ret && ret->init(filterType)) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

bool RangePopup::init(int filterType) {
	if (!Popup::init(200.0f, 120.0f)) return false;

	this->setID("range-menu");
	//this->setZOrder(102);
	this->setTitle(filterType == 0 ? "Custom length" : "Custom difficulty");

	m_closeBtn->setVisible(false);
	m_filterType = filterType;
	auto& saved = m_filterType == 0 ? g_levelFilters.customLengthFilter : g_levelFilters.customDiffFilter;

	auto label = CCLabelBMFont::create("From         to        ", "bigFont.fnt");
	label->setScale(0.45f);
	label->setPosition({ 100.0f, 64.0f });
	m_mainLayer->addChild(label);

	m_fromTextInput = TextInput::create(45.0f, "");
	m_fromTextInput->setPosition({ 83.0f, 64.0f });
	m_fromTextInput->setMaxCharCount(4);
	m_fromTextInput->setCommonFilter(CommonFilter::Uint);
	m_fromTextInput->setString(saved[0] != 0 ? std::to_string(saved[0]) : "");

	m_toTextInput = TextInput::create(45.0f, "");
	m_toTextInput->setPosition({ 161.0f, 64.0f });
	m_toTextInput->setMaxCharCount(4);
	m_toTextInput->setCommonFilter(CommonFilter::Uint);
	m_toTextInput->setString(saved[1] != 0 ? std::to_string(saved[1]) : "");

	auto okSpr = ButtonSprite::create("Ok");
	auto okBtn = CCMenuItemExt::createSpriteExtra(okSpr, [this, filterType](auto) {
		int from = std::stoi(m_fromTextInput->getString().size() != 0 ? m_fromTextInput->getString() : "0");
		int to = std::stoi(m_toTextInput->getString().size() != 0 ? m_toTextInput->getString() : "0");
		auto& saved = m_filterType == 0 ? g_levelFilters.customLengthFilter : g_levelFilters.customDiffFilter;

		if (filterType == 0) saved = {from, to};
		else saved = {from, to};

		Popup::onClose(this);
	});
	okBtn->setPosition({ 100.0f, 24.0f });

	m_buttonMenu->addChild(m_fromTextInput);
	m_buttonMenu->addChild(m_toTextInput);
	m_buttonMenu->addChild(okBtn);

	return true;
}

void RangePopup::onClose(cocos2d::CCObject*) {
	int from = std::stoi(m_fromTextInput->getString().size() != 0 ? m_fromTextInput->getString() : "0");
	int to = std::stoi(m_toTextInput->getString().size() != 0 ? m_toTextInput->getString() : "0");

	auto& saved = m_filterType == 0 ? g_levelFilters.customLengthFilter : g_levelFilters.customDiffFilter;
	if (from != saved[0] || to != saved[1]) {
		createQuickPopup("Hey!", "Fields not saved. Save and close?", "No", "Yes", [this, &saved, from, to](auto, bool yesBtn) {
			if (yesBtn) {
				saved = { from, to };
				Popup::onClose(this);
			}
		});
	}
	else Popup::onClose(this);
}