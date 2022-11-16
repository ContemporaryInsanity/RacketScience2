#include "plugin.hpp"

#include "RS.hpp"

struct RSSlew : Module {
	enum ParamIds {
		SLEW_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT,
		GATE,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	// With thanks to Paul https://github.com/baconpaul/BaconPlugs/blob/main/src/Glissinator.hpp
	float priorValue, targetValue;
	int offsetCount;

	RSSlew() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(SLEW_KNOB, 0.0f, 1.0f, 0.0f, "Slew", " S");
		configBypass(INPUT, OUTPUT);

		offsetCount = -1;
	}

	void process(const ProcessArgs& args) override {
		Module *m = this;
		if(!m) return;

		float currentValue = inputs[INPUT].getVoltage();
		float slewTime = params[SLEW_KNOB].getValue();

		int shiftTime = slewTime * args.sampleRate;
		if(shiftTime < 10) shiftTime = 10;

		if(offsetCount < 0) {
			priorValue = currentValue;
			offsetCount = 0;
		}

		bool slewing = offsetCount != 0;
		float outputValue = currentValue;

		if(offsetCount >= shiftTime) {
			offsetCount = 0;
			priorValue = currentValue;
			targetValue = currentValue;
			slewing = false;
		}

		if(!slewing) {
			if(currentValue != priorValue) {
				targetValue = currentValue;
				offsetCount = 1;
				slewing = true;
			}
		}

		if(slewing) {
			if(currentValue != targetValue) {
				float lastKnown = ((shiftTime - (offsetCount - 1)) * 
				priorValue + (offsetCount - 1) * targetValue) / shiftTime;
			targetValue =currentValue;
			priorValue = lastKnown;
			offsetCount = 0;
			}

			outputValue = ((shiftTime - offsetCount) * priorValue + offsetCount * currentValue) / shiftTime;
			offsetCount++;
		}
		
		outputs[OUTPUT].setVoltage(outputValue);
		outputs[GATE].setVoltage(slewing ? 10.0f : 0.0f);
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


struct RSSlewWidget : ModuleWidget {
    RSSlew *module;

	RSSlewWidget(RSSlew* module) {
		setModule(module);
        this->module = module;

        box.size.x = mm2px(5.08 * 3);
		int middle = box.size.x / 2 + 1;

		addChild(new RSLabelCentered(middle, box.pos.y + 15, "SLEW", RS_TITLE_FONT_SIZE, module));

		addChild(new RSLabelCentered(middle, box.size.y - 17, "Racket", RS_TITLE_FONT_SIZE, module));
		addChild(new RSLabelCentered(middle, box.size.y - 5, "Science", RS_TITLE_FONT_SIZE, module));

		addInput(createInputCentered<RSJackMonoIn>(Vec(middle, RS_ROW_COMP(0)), module, RSSlew::INPUT));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(0), "IN", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSKnobSml>(Vec(middle, RS_ROW_COMP(1)), module, RSSlew::SLEW_KNOB));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(1), "SLEW", RS_LABEL_FONT_SIZE, module));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(middle,  RS_ROW_COMP(2)), module, RSSlew::OUTPUT));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(2), "OUT", RS_LABEL_FONT_SIZE, module));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(middle,  RS_ROW_COMP(3)), module, RSSlew::GATE));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(3), "GATE", RS_LABEL_FONT_SIZE, module));
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
	};
};


Model* modelRSSlew = createModel<RSSlew, RSSlewWidget>("RSSlew");
