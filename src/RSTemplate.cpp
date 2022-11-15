#include "plugin.hpp"

#include "RS.hpp"

struct RSTemplate : Module {
	enum ParamIds {

		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	RSTemplate() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	}

	void process(const ProcessArgs& args) override {
		Module *m = this;
		if(!m) return;
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

	void contextMenu() {

	}
};


struct RSTemplateWidget : ModuleWidget {
    RSTemplate *module;

	RSTemplateWidget(RSTemplate* module) {
		setModule(module);
        this->module = module;

		int vs = 45, lo = 25; // Vertical spacing / label offset
        box.size.x = mm2px(5.08 * 3);
		int middle = box.size.x / 2 + 1;

		addChild(new RSLabelCentered(middle, box.pos.y + 15, "TITLE", 14, module));

		addChild(new RSLabelCentered(middle, box.size.y - 17, "Racket", 14, module));
		addChild(new RSLabelCentered(middle, box.size.y - 5, "Science", 14, module));

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


void appendContextMenu(Menu* menu) override {
		RSTemplate* module = dynamic_cast<RSTemplate*>(this->module);
		assert(module);

		menu->addChild(new MenuSeparator);

		menu->addChild(createMenuItem("Context menu", "",
			[=]() {module->contextMenu();}
		));
	}
};


Model* modelRSTemplate = createModel<RSTemplate, RSTemplateWidget>("RSTemplate");
