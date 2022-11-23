#define COLOR_RS_GREY   nvgRGB(0x19, 0x19, 0x19)
#define COLOR_RS_BRONZE nvgRGB(0x85, 0x87, 0x39)
#define COLOR_RS_LABEL  nvgRGB(0xf0, 0xf0, 0xf0)

#define RS_LAYOUT_TOP			36
#define RS_LAYOUT_V_SPACING 	41
#define RS_LAYOUT_LABEL_OFFSET	22
#define RS_ROW_COMP(r)	RS_LAYOUT_TOP + (RS_LAYOUT_V_SPACING * r)
#define RS_ROW_LABEL(r)	RS_LAYOUT_TOP + (RS_LAYOUT_V_SPACING * r) + RS_LAYOUT_LABEL_OFFSET

#define RS_TITLE_FONT_SIZE 14
#define RS_LABEL_FONT_SIZE 11

// Labels
struct RSLabel : LedDisplay {
	int fontSize;
	std::shared_ptr<Font> font;
	std::string text;
	NVGcolor color;

	RSLabel(int x, int y, const char* str = "", int fontSize = 10, const NVGcolor& colour = COLOR_RS_GREY) {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
		box.pos = Vec(x, y);
		box.size = Vec(120, 12);
		text = str;
		color = colour;
		this->fontSize = fontSize;
	}

	void draw(const DrawArgs &args) override {
		if(font->handle >= 0) {
			bndSetFont(font->handle);

			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, color);
			nvgText(args.vg, 0, 0, text.c_str(), NULL);
			nvgStroke(args.vg);

			bndSetFont(APP->window->uiFont->handle);
		}
	}
};

struct RSLabelCentered : LedDisplay {
	int fontSize;
	std::shared_ptr<Font> font;
	std::string text;
	int *themeIdx = NULL;

	RSLabelCentered(int x, int y, const char* str = "", int fontSize = 12, Module *module = NULL) {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
		this->fontSize = fontSize;
		box.pos = Vec(x, y);
		text = str;
	}

	void draw(const DrawArgs &args) override {
		if(font->handle >= 0) {
			bndSetFont(font->handle);

			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, COLOR_RS_LABEL);
			nvgText(args.vg, 0, 0, text.c_str(), NULL);
			nvgStroke(args.vg);

			bndSetFont(APP->window->uiFont->handle);
		}
	}
};

// Knobs
struct RSKnob : SVGKnob {
	RSKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		shadow->opacity = 0.0f;
	}
};

struct RSKnobDetent : RSKnob {
	RSKnobDetent() {
		snap = true;
	}
};

struct RSKnobSml : RSKnob { RSKnobSml() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobSml.svg"))); } };
struct RSKnobMed : RSKnob { RSKnobMed() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobMed.svg"))); } };
struct RSKnobLrg : RSKnob { RSKnobLrg() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobLrg.svg"))); } };

struct RSKnobInvisible : RSKnob { RSKnobInvisible() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobInvisible.svg"))); } };

struct RSKnobDetentSml : RSKnobDetent { RSKnobDetentSml() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobSml.svg"))); } };
struct RSKnobDetentMed : RSKnobDetent { RSKnobDetentMed() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobMed.svg"))); } };
struct RSKnobDetentLrg : RSKnobDetent { RSKnobDetentLrg() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobLrg.svg"))); } };

struct RSKnobDetentInvisible : RSKnobDetent { RSKnobDetentInvisible() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobInvisible.svg"))); } };


// Buttons
struct RSButton : SVGSwitch {
	RSButton() {
		shadow->opacity = 0.0f;
	}
};

struct RSButtonToggle : RSButton {
	RSButtonToggle() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButton.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButtonPress.svg")));
	}
};

struct RSRoundButtonToggle : RSButton {
	RSRoundButtonToggle() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSRoundButton.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSRoundButtonPress.svg")));
	}
};

struct RSButtonToggleInvisible : RSButton {
	RSButtonToggleInvisible() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButtonInvisibleIsh.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButtonInvisible.svg")));
	}
};

struct RSButtonMomentary : RSButtonToggle {
	RSButtonMomentary() {
		momentary = true;
	}
};

struct RSRoundButtonMomentary : RSRoundButtonToggle {
	RSRoundButtonMomentary() {
		momentary = true;
	}
};

struct RSButtonMomentaryInvisible : RSButtonToggleInvisible {
	RSButtonMomentaryInvisible() {
		momentary = true;
	}
};

// Switches
struct RSSwitch2P : SvgSwitch {
	RSSwitch2P() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_2.svg")));

		shadow->opacity = 0.0f;
	}

	void onChange(const event::Change &e) override {
		SvgSwitch::onChange(e);

        ParamQuantity* paramQuantity = getParamQuantity();
		if(paramQuantity->getValue() > 0.5f) paramQuantity->setValue(1.0f);
		else 								 paramQuantity->setValue(0.0f);
	}
};

struct RSSwitch3PV : SvgSwitch {
	RSSwitch3PV() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_2.svg")));

		shadow->opacity = 0.0f;
	}

	void onChange(const event::Change &e) override {
		SvgSwitch::onChange(e);

        ParamQuantity* paramQuantity = getParamQuantity();
		if(paramQuantity->getValue() > 1.33f) 	   paramQuantity->setValue(2.0f);
		else if(paramQuantity->getValue() > 0.67f) paramQuantity->setValue(1.0f);
		else 									   paramQuantity->setValue(0.0f);
	}
};

// Ports
struct RSJackMonoOut      : SVGPort { RSJackMonoOut()      { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoOut.svg"))); } };
struct RSJackSmallMonoOut : SVGPort { RSJackSmallMonoOut() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackSmallMonoOut.svg"))); } };
struct RSJackPolyOut      : SVGPort { RSJackPolyOut()      { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyOut.svg"))); } };
struct RSJackMonoIn       : SVGPort { RSJackMonoIn()       { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoIn.svg"))); } };
struct RSJackSmallMonoIn  : SVGPort { RSJackSmallMonoIn()  { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackSmallMonoIn.svg"))); } };
struct RSJackPolyIn       : SVGPort { RSJackPolyIn()       { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyIn.svg"))); } };

struct RSStealthJackIn : app::SvgPort {
	void step() override {
		if(!module) return;

		if(module->inputs[portId].isConnected()) {
			Widget::show();
		}
		else {
			CableWidget* cw = APP->scene->rack->getIncompleteCable();
			if(cw) {
				if(cw->outputPort) Widget::show();
				else Widget::hide();
			}
			else Widget::hide();
		}
		Widget::step();
	}
};

struct RSStealthJackOut : app::SvgPort {
	void step() override {
		if(!module) return;

		if(module->outputs[portId].isConnected()) {
			Widget::show();
		}
		else {
			CableWidget* cw = APP->scene->rack->getIncompleteCable();
			if(cw) {
				if(cw->inputPort) Widget::show();
				else Widget::hide();
			}
			else Widget::hide();
		}
		Widget::step();
	}
};

struct RSStealthJackMonoIn : RSStealthJackIn {
	RSStealthJackMonoIn() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoIn.svg")));
	}
};

struct RSStealthJackSmallMonoIn : RSStealthJackIn {
	RSStealthJackSmallMonoIn() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackSmallMonoIn.svg")));
	}
};

struct RSStealthJackMonoOut : RSStealthJackOut {
	RSStealthJackMonoOut() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoOut.svg")));
	}
};

struct RSStealthJackSmallMonoOut : RSStealthJackOut {
	RSStealthJackSmallMonoOut() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackSmallMonoOut.svg")));
	}
};

struct RSStealthJackPolyIn : RSStealthJackIn {
	RSStealthJackPolyIn() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyIn.svg")));
	}
};

struct RSStealthJackPolyOut : RSStealthJackOut {
	RSStealthJackPolyOut() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyOut.svg")));
	}
};

