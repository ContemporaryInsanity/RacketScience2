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
	float priorValue[16], targetValue[16];
	int offsetCount[16] = {-1};

	RSSlew() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configInput(INPUT, "CV to slew");
		configParam(SLEW_KNOB, 0.0f, 1.0f, 0.0f, "Slew", " S");
		configOutput(OUTPUT, "Slewed");
		configOutput(GATE, "High when slewing");
		configBypass(INPUT, OUTPUT);
	}

	void process(const ProcessArgs& args) override {
		Module *m = this;
		if(!m) return;

		int channelCount = inputs[INPUT].getChannels();
		outputs[OUTPUT].setChannels(channelCount);
		outputs[GATE].setChannels(channelCount);

		float slewTime = params[SLEW_KNOB].getValue();
		int shiftTime = slewTime * args.sampleRate;
		if(shiftTime < 10) shiftTime = 10;

		for(int channel = 0; channel < channelCount; channel++) {
			float currentValue = inputs[INPUT].getVoltage(channel);

			if(offsetCount[channel] < 0) {
				priorValue[channel] = currentValue;
				offsetCount[channel] = 0;
			}

			bool slewing = offsetCount[channel] != 0;
			float outputValue = currentValue;

			if(offsetCount[channel] >= shiftTime) {
				offsetCount[channel] = 0;
				priorValue[channel] = currentValue;
				targetValue[channel] = currentValue;
				slewing = false;
			}

			if(!slewing) {
				if(currentValue != priorValue[channel]) {
					targetValue[channel] = currentValue;
					offsetCount[channel] = 1;
					slewing = true;
				}
			}

			if(slewing) {
				if(currentValue != targetValue[channel]) {
					float lastKnown = ((shiftTime - (offsetCount[channel] - 1)) * priorValue[channel] + 
						(offsetCount[channel] - 1) * targetValue[channel]) / shiftTime;
					targetValue[channel] = currentValue;
					priorValue[channel] = lastKnown;
					offsetCount[channel] = 0;
				}

				outputValue = ((shiftTime - offsetCount[channel]) * priorValue[channel] + 
					offsetCount[channel] * currentValue) / shiftTime;
				offsetCount[channel]++;
			}
			
			outputs[OUTPUT].setVoltage(outputValue, channel);
			outputs[GATE].setVoltage(slewing ? 10.0f : 0.0f, channel);
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

		addInput(createInputCentered<RSJackPolyIn>(Vec(middle, RS_ROW_COMP(0)), module, RSSlew::INPUT));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(0), "IN", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSKnobSml>(Vec(middle, RS_ROW_COMP(1)), module, RSSlew::SLEW_KNOB));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(1), "SLEW", RS_LABEL_FONT_SIZE, module));

		addOutput(createOutputCentered<RSJackPolyOut>(Vec(middle,  RS_ROW_COMP(2)), module, RSSlew::OUTPUT));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(2), "OUT", RS_LABEL_FONT_SIZE, module));

		addOutput(createOutputCentered<RSJackPolyOut>(Vec(middle,  RS_ROW_COMP(3)), module, RSSlew::GATE));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(3), "GATE", RS_LABEL_FONT_SIZE, module));
	};

	#include "RSModuleWidgetDraw.hpp"

	void customDraw(const DrawArgs& args) {}
};


Model* modelRSSlew = createModel<RSSlew, RSSlewWidget>("RSSlew");
