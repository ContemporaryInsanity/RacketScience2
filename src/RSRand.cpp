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
		
		ModuleWidget *mw = APP->scene->rack->getModule(m->rightExpander.moduleId);
		if(!mw) return;

		if(randTrigger.process(params[RAND_BUTTON].getValue() + inputs[RAND_INPUT].getVoltage())) {
			int i = 0;

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

		int vs = 45, lo = 25; // Vertical spacing / label offset
        box.size.x = mm2px(5.08 * 3);
		int middle = box.size.x / 2 + 1;

		addChild(new RSLabelCentered(middle, box.pos.y + 15, "RAND>", 14, module));

		addChild(new RSLabelCentered(middle, box.size.y - 17, "Racket", 14, module));
		addChild(new RSLabelCentered(middle, box.size.y - 5, "Science", 14, module));

		addParam(createParamCentered<RSButtonMomentary>(Vec(middle, vs), module, RSRand::RAND_BUTTON));
		addChild(new RSLabelCentered(middle, vs + lo, "RAND", 12, module));

		addParam(createParamCentered<RSKnobSml>(Vec(middle, vs * 2), module, RSRand::RAND_KNOB));
		addChild(new RSLabelCentered(middle, vs * 2 + lo, "%", 12, module));

		addParam(createParamCentered<RSKnobSml>(Vec(middle, vs * 3), module, RSRand::SLEW_KNOB));
		addChild(new RSLabelCentered(middle, vs * 3 + lo, "SLEW", 12, module));

		addParam(createParamCentered<RSButtonToggle>(Vec(middle, vs * 4), module, RSRand::PIVOT_BUTTON));
		addChild(new RSLabelCentered(middle, vs * 4 + lo, "PIVOT", 12, module));

		addParam(createParamCentered<RSRoundButtonToggle>(Vec(middle, vs * 5), module, RSRand::THINGY_BUTTON));
		addChild(new RSLabelCentered(middle, vs * 5 + lo, "THINGY", 12, module));

		addParam(createParamCentered<RSRoundButtonToggle>(Vec(middle, vs * 6), module, RSRand::WOTSIT_BUTTON));
		addChild(new RSLabelCentered(middle, vs * 6 + lo, "WOTSIT", 12, module));

		addInput(createInputCentered<RSJackMonoIn>(Vec(middle, vs * 7), module, RSRand::RAND_INPUT));
		addChild(new RSLabelCentered(middle, vs * 7 + lo, "TRIG", 12, module));

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