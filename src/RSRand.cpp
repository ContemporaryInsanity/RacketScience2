#include "plugin.hpp"

#include "RS.hpp"

struct RSRand : Module {
	enum ParamIds {
		RAND_BUTTON,
		RAND_KNOB,
		SLEW_KNOB,
		PIVOT_BUTTON,
		THINGY_BUTTON,
		WOTSIT_BUTTON,
		EJECT_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		RAND_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1, OUT2,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT1,
		NUM_LIGHTS
	};

	dsp::SchmittTrigger randTrigger;
	dsp::SchmittTrigger pivotTrigger;

	std::vector<float> f;

	RSRand() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(RAND_BUTTON, 0.0f, 1.0f, 0.0f, "Randomise");
		configParam(RAND_KNOB, 0.0f, 1.0f, 0.5f, "Randomisation percentage", "%", 0.0f, 100.0f);
		configParam(SLEW_KNOB, 0.0f, 1.0f, 0.0f, "Slew", " S");
		configParam(PIVOT_BUTTON, 0.0f, 1.0f, 0.0f, "Pivot");
	}

	void process(const ProcessArgs& args) override {
		Module *m = this;
		if(!m) return;
        
		if(m->rightExpander.moduleId < 0) return;
		

		if(randTrigger.process(params[RAND_BUTTON].getValue() + inputs[RAND_INPUT].getVoltage())) {
			int i = 0;
			
			ModuleWidget *mw = APP->scene->rack->getModule(m->rightExpander.moduleId);
			if(!mw) return;

			for(ParamWidget *param : mw->getParams()) {
				float r = (float)rand() / (float)RAND_MAX - 0.5f;
				
				float v;
                // If PIVOTing
				if(params[PIVOT_BUTTON].getValue() && !f.empty())		// SLEW: v = in
					v = f[i++];	// used previously stored parameters
				else			// else use live parameters
					v = param->getParamQuantity()->getScaledValue();

				float m = params[RAND_KNOB].getValue();
				float vr = std::max(0.0f, std::min(v + (r * m), 1.0f));	// SLEW: vr is targetIn
				param->getParamQuantity()->setScaledValue(vr);
			}
		}

		// On hitting PIVOT, store current parameters
        if(pivotTrigger.process(params[PIVOT_BUTTON].getValue())) {
			f.clear();

			ModuleWidget *mw = APP->scene->rack->getModule(m->rightExpander.moduleId);
			if(!mw) return;

			for(ParamWidget *param : mw->getParams())
				f.push_back(param->getParamQuantity()->getScaledValue());
		}
	}

	void onReset() override {


    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();


        return rootJ;
    }


	void dataFromJson(json_t* rootJ) override {
		// json_t* ?J = json_object_get(rootJ, "?");


	}
};


struct RSRandWidget : ModuleWidget {
    RSRand *module;

	RSRandWidget(RSRand* module) {
		setModule(module);
        this->module = module;

        box.size.x = mm2px(5.08 * 3);
		int middle = box.size.x / 2 + 1;

		addChild(new RSLabelCentered(middle, box.pos.y + 15, "RAND>", RS_TITLE_FONT_SIZE, module));

		addChild(new RSLabelCentered(middle, box.size.y - 17, "Racket", RS_TITLE_FONT_SIZE, module));
		addChild(new RSLabelCentered(middle, box.size.y - 5, "Science", RS_TITLE_FONT_SIZE, module));

		addParam(createParamCentered<RSButtonMomentary>(Vec(middle, RS_ROW_COMP(0)), module, RSRand::RAND_BUTTON));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(0), "RAND", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSKnobSml>(Vec(middle, RS_ROW_COMP(1)), module, RSRand::RAND_KNOB));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(1), "%", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSKnobSml>(Vec(middle, RS_ROW_COMP(2)), module, RSRand::SLEW_KNOB));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(2), "SLEW", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSButtonToggle>(Vec(middle, RS_ROW_COMP(3)), module, RSRand::PIVOT_BUTTON));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(3), "PIVOT", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSRoundButtonToggle>(Vec(middle, RS_ROW_COMP(4)), module, RSRand::THINGY_BUTTON));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(4), "THINGY", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSRoundButtonToggle>(Vec(middle, RS_ROW_COMP(5)), module, RSRand::WOTSIT_BUTTON));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(5), "WOTSIT", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSRoundButtonToggle>(Vec(middle, RS_ROW_COMP(6)), module, RSRand::WOTSIT_BUTTON));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(6), "EJECT", RS_LABEL_FONT_SIZE, module));

		addInput(createInputCentered<RSJackMonoIn>(Vec(middle, RS_ROW_COMP(7)), module, RSRand::RAND_INPUT));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(7), "TRIG", RS_LABEL_FONT_SIZE, module));
	};


	void customDraw(const DrawArgs& args) {}
	//#include "RSModuleWidgetDraw.hpp"

	// above include contents:
	// Draws panel background instead of using SVG file
	// Calls customDraw() should we need to draw anything else

	void draw(const DrawArgs& args) override {
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		nvgFillColor(args.vg, COLOR_RS_GREY);

		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 1, 1, box.size.x - 1, box.size.y - 1, 5);
		nvgStroke(args.vg);
		nvgFill(args.vg);

		customDraw(args);

		ModuleWidget::draw(args);
	}
};


Model* modelRSRand = createModel<RSRand, RSRandWidget>("RSRand");
