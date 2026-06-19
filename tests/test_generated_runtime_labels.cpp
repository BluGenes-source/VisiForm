#include "generator/VisageCppEmitter.h"
#include "model/ProjectDocument.h"
#include "model/WidgetRegistry.h"

#include <catch2/catch_test_macros.hpp>

using visiform::generator::VisageCppEmitter;
using visiform::model::ProjectDocument;
using visiform::model::WidgetNode;
using visiform::model::WidgetRegistry;
using visiform::model::WidgetType;

namespace {

WidgetNode makeNamedWidget(WidgetType type, const std::string& id, const std::string& name)
{
    WidgetNode widget = WidgetRegistry::instance().createDefaultWidget(type, id);
    widget.name = name;
    return widget;
}

}

TEST_CASE("generated runtime keeps internal widget names out of visible text")
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.root.children.clear();

    WidgetNode slider = makeNamedWidget(WidgetType::Slider, "slider_id", "internalSliderName");

    WidgetNode button = makeNamedWidget(WidgetType::Button, "button_id", "internalButtonName");
    button.setProperty("text", "Visible Button");
    button.setProperty("normalText", "");

    WidgetNode label = makeNamedWidget(WidgetType::Label, "label_id", "internalLabelName");
    label.setProperty("text", "");

    WidgetNode frame = makeNamedWidget(WidgetType::Frame, "frame_id", "internalFrameName");
    frame.setProperty("title", "");

    WidgetNode dialog = makeNamedWidget(WidgetType::ModalDialog, "dialog_id", "internalDialogName");
    dialog.setProperty("title", "");

    document.root.children = { slider, button, label, frame, dialog };
    document.refreshHierarchyMetadata();

    VisageCppEmitter::EmittedSources output;
    std::string errorMessage;
    REQUIRE(VisageCppEmitter{}.emitProjectSources(document, {}, output, errorMessage));
    REQUIRE(errorMessage.empty());

    const std::string& generated = output.generatedBaseCpp;
    REQUIRE(generated.find("widget.name = \"internalSliderName\";") != std::string::npos);
    REQUIRE(generated.find("widget.name = \"internalButtonName\";") != std::string::npos);
    REQUIRE(generated.find("widget.name = \"internalLabelName\";") != std::string::npos);
    REQUIRE(generated.find("widget.name = \"internalFrameName\";") != std::string::npos);
    REQUIRE(generated.find("widget.name = \"internalDialogName\";") != std::string::npos);

    REQUIRE(generated.find("widget.text.value = \"internalSliderName\";") == std::string::npos);
    REQUIRE(generated.find("widget.text.value = \"internalButtonName\";") == std::string::npos);
    REQUIRE(generated.find("widget.text.value = \"internalLabelName\";") == std::string::npos);
    REQUIRE(generated.find("widget.text.value = \"internalFrameName\";") == std::string::npos);
    REQUIRE(generated.find("widget.dialogTitle = \"internalDialogName\";") == std::string::npos);
    REQUIRE(generated.find("widget.text.value = \"Visible Button\";") != std::string::npos);
    REQUIRE(generated.find("widget.button.normalText = \"Visible Button\";") != std::string::npos);
    REQUIRE(generated.find("modalState_.title = widget->dialogTitle;") != std::string::npos);
}

TEST_CASE("generated runtime emits the selected look and feel state palette")
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.lookAndFeelId = "VisiFormLight";

    VisageCppEmitter::EmittedSources output;
    std::string errorMessage;
    REQUIRE(VisageCppEmitter{}.emitProjectSources(document, {}, output, errorMessage));
    REQUIRE(errorMessage.empty());

    const std::string& generated = output.generatedBaseCpp;
    CHECK(generated.find("widget.style.recessedColor = makeColor(0xE8, 0xEC, 0xF2);") != std::string::npos);
    CHECK(generated.find("widget.style.hoverColor = makeColor(0xED, 0xF4, 0xFF);") != std::string::npos);
    CHECK(generated.find("widget.style.pressedColor = makeColor(0xDC, 0xE4, 0xEF);") != std::string::npos);
    CHECK(generated.find("widget.style.highlightColor = makeColor(0xFF, 0xFF, 0xFF);") != std::string::npos);
    CHECK(generated.find("return widget.style.hoverColor;") != std::string::npos);
    CHECK(generated.find("widget.style.focusColor") != std::string::npos);
}

TEST_CASE("generated runtime emits project look and feel overrides")
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.lookAndFeelId = "VisiFormLight";
    document.lookAndFeelOverrides.controlSurfaceColor = "#123456";
    document.lookAndFeelOverrides.focusOutlineColor = "#654321";
    document.lookAndFeelOverrides.cornerRadius = 9.0f;

    VisageCppEmitter::EmittedSources output;
    std::string errorMessage;
    REQUIRE(VisageCppEmitter{}.emitProjectSources(document, {}, output, errorMessage));
    REQUIRE(errorMessage.empty());

    const std::string& generated = output.generatedBaseCpp;
    CHECK(generated.find("widget.style.fillColor = makeColor(0x12, 0x34, 0x56);") != std::string::npos);
    CHECK(generated.find("widget.style.focusColor = makeColor(0x65, 0x43, 0x21);") != std::string::npos);
    CHECK(generated.find("widget.style.cornerRadius = 9.00f;") != std::string::npos);
}
