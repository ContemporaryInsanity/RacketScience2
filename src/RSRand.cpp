#include "plugin.hpp"

#include "RS.hpp"

struct RSRand : Module
{
	enum ParamIds
	{
		RAND_BUTTON,
		RAND_KNOB,
		SLEW_KNOB,
		PIVOT_BUTTON,
		FREEZE_BUTTON,
		FORCE_BUTTON,
		EXCLUDE_BUTTON,
		NUM_PARAMS
	};
	enum InputIds
	{
		RAND_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		NUM_OUTPUTS
	};
	enum LightIds
	{
		SLEW_LIGHT,
		NUM_LIGHTS
	};

	dsp::ClockDivider modDivider;
	int modDiv = 32;

	dsp::SchmittTrigger randTrigger;
	dsp::SchmittTrigger pivotTrigger;

	// Right module ID tracking
	int64_t RMId = -1;
	int64_t priorRMId = -2;

	// For PIVOTing
	std::vector<float> storedValue;

	// For SLEWing
	std::vector<float> currentValue;
	std::vector<float> priorValue;
	std::vector<float> targetValue;
	std::vector<int> offsetCount;

	// Options
	bool freeze;
	bool force;
	bool exclude;

	RSRand()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configButton(RAND_BUTTON, "Randomise");
		configParam(RAND_KNOB, 0.0f, 1.0f, 0.5f, "Randomisation percentage", "%", 0.0f, 100.0f);
		configParam(SLEW_KNOB, 0.0f, 5.0f, 0.0f, "Slew", " S");
		configSwitch(PIVOT_BUTTON, 0.0f, 1.0f, 0.0f, "Pivot", {"OFF", "ON"});
		configInput(RAND_INPUT, "Randomisation trigger");

		// freeze force exclude

		modDivider.setDivision(modDiv);
	}

	void process(const ProcessArgs &args) override
	{
		Module *m = this;
		if (!m)
			return;


		if (modDivider.process())
		{
			RMId = m->rightExpander.moduleId;

			ModuleWidget *mw = APP->scene->rack->getModule(RMId);
			if (!mw)
				return;

			if (RMId != priorRMId)
			{ // We're initialising or have a new right module
				printf("RSRand:RMId changed\n");
				priorRMId = RMId;
				if (RMId < 0)
					return;

				mw = APP->scene->rack->getModule(RMId);
				if (!mw)
					return;

				// How many parameters are we dealing with?
				int params = (int)(mw->getParams().size());
				printf("RSRand:%i params\n", params);

				// Initialise vectors
				currentValue.clear();
				currentValue.reserve(params);
				priorValue.clear();
				priorValue.reserve(params);
				targetValue.clear();
				targetValue.reserve(params);
				offsetCount.clear();
				offsetCount.reserve(params);

				int i = 0;
				for (ParamWidget *param : mw->getParams())
				{
					offsetCount.push_back(-1);
					currentValue[i++] = param->getParamQuantity()->getScaledValue();
				}
			}

			// This probably needs to go outside of the divider, inside we can miss triggers depending on length & divider setting
			if (randTrigger.process(params[RAND_BUTTON].getValue() + inputs[RAND_INPUT].getVoltage()))
			{
				int i = 0;
				for (ParamWidget *param : mw->getParams())
				{
					if (params[PIVOT_BUTTON].getValue() && !storedValue.empty()) // If PIVOTing
						currentValue[i] = storedValue[i];						 // use previously stored parameters
					else														 // else use live parameters and get a bonus random walk for free
						currentValue[i] = param->getParamQuantity()->getScaledValue();

					float r = (float)rand() / (float)RAND_MAX - 0.5f;
					currentValue[i] = std::max(0.0f, std::min(currentValue[i] + (r * params[RAND_KNOB].getValue()), 1.0f));

					if (!params[SLEW_KNOB].getValue() && param->getParamQuantity()->randomizeEnabled)
						param->getParamQuantity()->setScaledValue(currentValue[i]);

					i++;
				}
			}

			if (pivotTrigger.process(params[PIVOT_BUTTON].getValue()))
			{
				printf("RSRand:PIVOT\n");

				storedValue.clear();
				for (ParamWidget *param : mw->getParams())
					storedValue.push_back(param->getParamQuantity()->getScaledValue());
			}

			freeze = params[FREEZE_BUTTON].getValue() ? true : false;
			force = params[FORCE_BUTTON].getValue() ? true : false;

			float slewTime = params[SLEW_KNOB].getValue();
			int shiftTime = slewTime * args.sampleRate / modDiv;

			int i = 0;

			if (!freeze)
			{
				for (ParamWidget *param : mw->getParams())
				{

					// As we are NOT setting this on a trigger like before, Stoermelder GRIPs are being overridden
					// Setting GRIP to audio rate processing appears to alleviate this

					if (offsetCount[i] < 0)
					{
						priorValue[i] = currentValue[i];
						offsetCount[i] = 0;
					}

					bool slewing = offsetCount[i] != 0;
					float outputValue = currentValue[i];

					if (offsetCount[i] >= shiftTime)
					{
						offsetCount[i] = 0;
						priorValue[i] = currentValue[i];
						targetValue[i] = currentValue[i];
						slewing = false;
					}

					if (!slewing)
					{
						if (currentValue[i] != priorValue[i])
						{
							targetValue[i] = currentValue[i];
							offsetCount[i] = 0;
							slewing = true;
						}
					}

					if (slewing)
					{
						if (currentValue[i] != targetValue[i])
						{
							float lastKnown = ((shiftTime - (offsetCount[i] - 1)) * priorValue[i] +
											   (offsetCount[i] - 1) * targetValue[i]) /
											  shiftTime;
							targetValue[i] = currentValue[i];
							priorValue[i] = lastKnown;
							offsetCount[i] = 0;
						}

						outputValue = ((shiftTime - offsetCount[i]) * priorValue[i] +
									   offsetCount[i] * currentValue[i]) /
									  shiftTime;

						offsetCount[i]++;
					}

					if (slewing && (param->getParamQuantity()->randomizeEnabled || force))
						param->getParamQuantity()->setScaledValue(outputValue); // Only update this when actually slewing?

					// As is we can't adjust knobs on target module when not slewing as we're constantly updating here.

					// Would be nice to have a light to indicate when we're slewing,
					//  this could help to set slew time when triggering rythmically,
					//	would a slew gate output be of any use?

					i++;
				}
			}
		}
	}

	void onReset() override
	{
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		// json_t* ?J = json_object_get(rootJ, "?");
	}
};

struct RSRandWidget : ModuleWidget
{
	RSRand *module;

	RSRandWidget(RSRand *module)
	{
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

		addParam(createParamCentered<RSButtonToggle>(Vec(middle, RS_ROW_COMP(4)), module, RSRand::FREEZE_BUTTON));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(4), "FREEZE", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSButtonToggle>(Vec(middle, RS_ROW_COMP(5)), module, RSRand::FORCE_BUTTON));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(5), "FORCE", RS_LABEL_FONT_SIZE, module));

		addParam(createParamCentered<RSButtonToggle>(Vec(middle, RS_ROW_COMP(6)), module, RSRand::EXCLUDE_BUTTON));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(6), "EXCLUDE", RS_LABEL_FONT_SIZE, module));

		addInput(createInputCentered<RSJackMonoIn>(Vec(middle, RS_ROW_COMP(7)), module, RSRand::RAND_INPUT));
		addChild(new RSLabelCentered(middle, RS_ROW_LABEL(7), "TRIG", RS_LABEL_FONT_SIZE, module));
	};

#include "RSModuleWidgetDraw.hpp"

	void customDraw(const DrawArgs &args)
	{
	}
};

Model *modelRSRand = createModel<RSRand, RSRandWidget>("RSRand");
