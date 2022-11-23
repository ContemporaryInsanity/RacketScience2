#include "plugin.hpp"

#include "RS.hpp"

struct RSRand : Module {
	enum ParamIds {
		RAND_BUTTON,
		RAND_KNOB,
		SLEW_KNOB,
		PIVOT_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		RAND_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT1,
		NUM_LIGHTS
	};

	dsp::ClockDivider modDivider;

	dsp::SchmittTrigger randTrigger;
	dsp::SchmittTrigger pivotTrigger;

	// Right module ID tracking
	int64_t RMId = -1;
	int64_t priorRMId = -1;

	// For PIVOTing
	std::vector<float> storedValue;

	// For SLEWing
	std::vector<float> currentValue;
	std::vector<float> priorValue;
	std::vector<float> targetValue;
	std::vector<int> offsetCount;

	RSRand() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configButton(RAND_BUTTON, "Randomise");
		configParam(RAND_KNOB, 0.0f, 1.0f, 0.5f, "Randomisation percentage", "%", 0.0f, 100.0f);
		configParam(SLEW_KNOB, 0.0f, 1.0f, 0.0f, "Slew", " S");
		configSwitch(PIVOT_BUTTON, 0.0f, 1.0f, 0.0f, "Pivot", {"OFF", "ON"});
		configInput(RAND_INPUT, "Randomisation trigger");

		modDivider.setDivision(512);
	}

	void process(const ProcessArgs& args) override {
		Module *m = this;
		if(!m) return;

		if(modDivider.process()) {
			RMId = m->rightExpander.moduleId;
			if(RMId < 0) return;

			ModuleWidget *mw = APP->scene->rack->getModule(RMId);
			if(!mw) return;
			
			if(RMId != priorRMId) { // We're initialising or have a new right module
				printf("RSRand:RMId changed\n");

				mw = APP->scene->rack->getModule(RMId);
				if(!mw) return;

				// How many parameters are we dealing with?
				int params = (int)(mw->getParams().size());
				printf("RSRand:%i params\n", params);
				
				// Initialise vectors
				currentValue.clear();	currentValue.reserve(params);
				priorValue.clear();		priorValue.reserve(params);
				targetValue.clear();	targetValue.reserve(params);
				offsetCount.clear();	offsetCount.reserve(params);

				int i = 0;
				for(ParamWidget *param : mw->getParams()) {
					offsetCount.push_back(-1);
					currentValue[i++] = param->getParamQuantity()->getScaledValue();
				}

				priorRMId = RMId;
			}

			float randMult = params[RAND_KNOB].getValue();

			if(randTrigger.process(params[RAND_BUTTON].getValue() + inputs[RAND_INPUT].getVoltage())) {

				int i = 0;
				for(ParamWidget *param : mw->getParams()) {
					if(params[PIVOT_BUTTON].getValue() && !storedValue.empty())	// If PIVOTing
						currentValue[i] = storedValue[i];	// used previously stored parameters
					else									// else use live parameters and get a bonus random walk for free
						currentValue[i] = param->getParamQuantity()->getScaledValue();

					float r = (float)rand() / (float)RAND_MAX - 0.5f;
					currentValue[i] = std::max(0.0f, std::min(currentValue[i] + (r * randMult), 1.0f));

					i++;
				}
			}

			// On hitting PIVOT, store current parameters
			if(pivotTrigger.process(params[PIVOT_BUTTON].getValue())) {
				printf("RSRand:PIVOT\n");

				storedValue.clear();

				for(ParamWidget *param : mw->getParams())
					storedValue.push_back(param->getParamQuantity()->getScaledValue());
			}

			float slewTime = params[SLEW_KNOB].getValue();
			int shiftTime = slewTime * args.sampleRate / 512;
			//if(shiftTime < 10) shiftTime = 10;
			
			int i = 0;
			for(ParamWidget *param : mw->getParams()) {

				// As we are NOT setting this on a trigger like before, Stoermelder GRIPs are being overridden 
				// Setting GRIP to audio rate processing appears to alleviate this

				if(offsetCount[i] < 0) {
					priorValue[i] = currentValue[i];
					offsetCount[i] = 0;
				}

				bool slewing = offsetCount[i] != 0;
				float outputValue = currentValue[i];

				if(offsetCount[i] >= shiftTime) {
					offsetCount[i] = 0;
					priorValue[i] = currentValue[i];
					targetValue[i] = currentValue[i];
					slewing = false;
				}

				if(!slewing) {
					if(currentValue[i] != priorValue[i]) {
						targetValue[i] = currentValue[i];
						offsetCount[i] = 0;
						slewing = true;
					}
				}

				if(slewing) {
					if(currentValue[i] != targetValue[i]) {
						float lastKnown = ((shiftTime - (offsetCount[i] - 1)) * priorValue[i] +
							(offsetCount[i] - 1) * targetValue[i]) / shiftTime;
						targetValue[i] = currentValue[i];
						priorValue[i] = lastKnown;
						offsetCount[i] = 0;
					}

					outputValue = ((shiftTime - offsetCount[i]) * priorValue[i] +
						offsetCount[i] * currentValue[i]) / shiftTime;

					offsetCount[i]++;
				}

				param->getParamQuantity()->setScaledValue(outputValue); // Only update this when actually slewing?
				// As is we can't adjust knobs on target module when not slewing as we're constantly updating here.

				// Would be nice to have a light to indicate when we're slewing,
				//  this could help to set slew time when triggering rythmically,
				//	would a slew gate output be of any use?

				i++;
			}
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
