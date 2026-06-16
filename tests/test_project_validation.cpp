#include "model/ProjectDocument.h"
#include "model/WidgetRegistry.h"
#include "utils/AppSettings.h"
#include "validation/ProjectValidator.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <utility>

using visiform::model::ProjectDocument;
using visiform::model::WidgetNode;
using visiform::model::WidgetRegistry;
using visiform::model::WidgetType;
using visiform::utils::AppSettings;
using visiform::validation::ProjectValidator;
using visiform::validation::ValidationMessage;
using visiform::validation::ValidationReport;
using visiform::validation::ValidationSeverity;

namespace {

ProjectDocument makeValidationDocument()
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.projectName = "SizerValidation";
    document.executableName = "SizerValidation";
    document.userSubclassName = "SizerValidationWindow";
    document.root.children.clear();
    document.selectedWidgetId = document.root.id;
    document.refreshHierarchyMetadata();
    return document;
}

WidgetNode makeWidget(WidgetType type, const std::string& id, float width, float height)
{
    WidgetNode widget = WidgetRegistry::instance().createDefaultWidget(type, id);
    widget.bounds.width = width;
    widget.bounds.height = height;
    return widget;
}

const ValidationMessage* findMessage(const ValidationReport& report, const char* code)
{
    for (const auto& message : report.messages) {
        if (message.code == code) {
            return &message;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("ProjectValidator reports invalid sizer properties and undersized bounds")
{
    ProjectDocument document = makeValidationDocument();

    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 40.0f, 20.0f);
    sizer.setProperty("orientation", "Diagonal");
    sizer.setProperty("paddingLeft", -2);
    sizer.setProperty("gap", "abc");

    auto button = makeWidget(WidgetType::Button, "button_1", 80.0f, 24.0f);
    sizer.appendChild(std::move(button));
    document.root.appendChild(std::move(sizer));
    document.refreshHierarchyMetadata();

    const ValidationReport report = ProjectValidator{}.validate(document, AppSettings{});

    const ValidationMessage* orientation = findMessage(report, "WIDGET_SIZER_ORIENTATION_INVALID");
    REQUIRE(orientation != nullptr);
    CHECK(orientation->severity == ValidationSeverity::Error);
    CHECK(orientation->widgetId == "sizer_main");
    CHECK(orientation->propertyKey == "orientation");

    const ValidationMessage* padding = findMessage(report, "WIDGET_SIZER_PADDING_NEGATIVE");
    REQUIRE(padding != nullptr);
    CHECK(padding->severity == ValidationSeverity::Error);
    CHECK(padding->propertyKey == "paddingLeft");

    const ValidationMessage* gap = findMessage(report, "WIDGET_SIZER_GAP_INVALID");
    REQUIRE(gap != nullptr);
    CHECK(gap->severity == ValidationSeverity::Error);
    CHECK(gap->propertyKey == "gap");

    const ValidationMessage* undersized = findMessage(report, "WIDGET_SIZER_UNDERSIZED");
    REQUIRE(undersized != nullptr);
    CHECK(undersized->severity == ValidationSeverity::Warning);
    CHECK(undersized->propertyKey == "bounds");
}

TEST_CASE("ProjectValidator reports direct sizer-child layout conflicts")
{
    ProjectDocument document = makeValidationDocument();

    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 240.0f, 140.0f);
    sizer.setProperty("orientation", "Vertical");

    auto button = makeWidget(WidgetType::Button, "button_1", 80.0f, 24.0f);
    button.setProperty("dock", "Fill");
    button.setProperty("anchor", "Bottom Right");
    button.setProperty("sizerItem.expand", true);
    button.setProperty("sizerItem.alignment", "End");
    button.setProperty("sizerItem.borderSides", "Left|Middle");
    button.setProperty("sizerItem.preferredHeight", -3);
    button.setProperty("sizerItem.minimumWidth", -2);

    sizer.appendChild(std::move(button));
    document.root.appendChild(std::move(sizer));
    document.refreshHierarchyMetadata();

    const ValidationReport report = ProjectValidator{}.validate(document, AppSettings{});

    const ValidationMessage* borderSides = findMessage(report, "WIDGET_SIZER_ITEM_BORDER_SIDES_INVALID");
    REQUIRE(borderSides != nullptr);
    CHECK(borderSides->severity == ValidationSeverity::Error);
    CHECK(borderSides->widgetId == "button_1");

    const ValidationMessage* preferredHeight = findMessage(report, "WIDGET_SIZER_ITEM_PREFERRED_INVALID");
    REQUIRE(preferredHeight != nullptr);
    CHECK(preferredHeight->severity == ValidationSeverity::Warning);
    CHECK(preferredHeight->propertyKey == "sizerItem.preferredHeight");

    const ValidationMessage* minimumWidth = findMessage(report, "WIDGET_SIZER_ITEM_MINIMUM_INVALID");
    REQUIRE(minimumWidth != nullptr);
    CHECK(minimumWidth->severity == ValidationSeverity::Warning);
    CHECK(minimumWidth->propertyKey == "sizerItem.minimumWidth");

    const ValidationMessage* alignmentIgnored = findMessage(report, "WIDGET_SIZER_ITEM_ALIGNMENT_IGNORED");
    REQUIRE(alignmentIgnored != nullptr);
    CHECK(alignmentIgnored->severity == ValidationSeverity::Warning);
    CHECK(alignmentIgnored->propertyKey == "sizerItem.alignment");

    const ValidationMessage* dockIgnored = findMessage(report, "WIDGET_SIZER_CHILD_DOCK_IGNORED");
    REQUIRE(dockIgnored != nullptr);
    CHECK(dockIgnored->severity == ValidationSeverity::Warning);
    CHECK(dockIgnored->propertyKey == "dock");

    const ValidationMessage* anchorIgnored = findMessage(report, "WIDGET_SIZER_CHILD_ANCHOR_IGNORED");
    REQUIRE(anchorIgnored != nullptr);
    CHECK(anchorIgnored->severity == ValidationSeverity::Warning);
    CHECK(anchorIgnored->propertyKey == "anchor");
}

TEST_CASE("ProjectValidator reports invalid spacer metadata")
{
    ProjectDocument document = makeValidationDocument();

    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 240.0f, 140.0f);
    sizer.setProperty("orientation", "Horizontal");

    auto spacer = makeWidget(WidgetType::Spacer, "spacer_1", 24.0f, 24.0f);
    spacer.setProperty("spacer.kind", "Elastic");
    spacer.setProperty("spacer.size", -5);

    sizer.appendChild(std::move(spacer));
    document.root.appendChild(std::move(sizer));
    document.refreshHierarchyMetadata();

    const ValidationReport report = ProjectValidator{}.validate(document, AppSettings{});

    const ValidationMessage* kind = findMessage(report, "WIDGET_SPACER_KIND_INVALID");
    REQUIRE(kind != nullptr);
    CHECK(kind->severity == ValidationSeverity::Error);
    CHECK(kind->widgetId == "spacer_1");
    CHECK(kind->propertyKey == "spacer.kind");

    const ValidationMessage* size = findMessage(report, "WIDGET_SPACER_SIZE_NEGATIVE");
    REQUIRE(size != nullptr);
    CHECK(size->severity == ValidationSeverity::Error);
    CHECK(size->widgetId == "spacer_1");
    CHECK(size->propertyKey == "spacer.size");
}

TEST_CASE("ProjectValidator reports invalid event callback identifiers")
{
    ProjectDocument document = makeValidationDocument();

    auto button = makeWidget(WidgetType::Button, "button_1", 120.0f, 32.0f);
    button.setProperty("onClick", "handle click");

    document.root.appendChild(std::move(button));
    document.refreshHierarchyMetadata();

    const ValidationReport report = ProjectValidator{}.validate(document, AppSettings{});

    const ValidationMessage* invalidCallback = findMessage(report, "CALLBACK_NAME_INVALID");
    REQUIRE(invalidCallback != nullptr);
    CHECK(invalidCallback->severity == ValidationSeverity::Error);
    CHECK(invalidCallback->widgetId == "button_1");
    CHECK(invalidCallback->propertyKey == "onClick");
}

TEST_CASE("ProjectValidator reports incompatible event callback signature reuse")
{
    ProjectDocument document = makeValidationDocument();

    auto button = makeWidget(WidgetType::Button, "button_1", 120.0f, 32.0f);
    button.setProperty("onClick", "handleShared");
    auto slider = makeWidget(WidgetType::Slider, "slider_1", 180.0f, 28.0f);
    slider.setProperty("onChanged", "handleShared");

    document.root.appendChild(std::move(button));
    document.root.appendChild(std::move(slider));
    document.refreshHierarchyMetadata();

    const ValidationReport report = ProjectValidator{}.validate(document, AppSettings{});

    const ValidationMessage* signatureConflict = findMessage(report, "CALLBACK_SIGNATURE_CONFLICT");
    REQUIRE(signatureConflict != nullptr);
    CHECK(signatureConflict->severity == ValidationSeverity::Error);
    CHECK(signatureConflict->widgetId == "slider_1");
    CHECK(signatureConflict->propertyKey == "onChanged");
}

TEST_CASE("ProjectValidator allows compatible event callback reuse")
{
    ProjectDocument document = makeValidationDocument();

    auto firstButton = makeWidget(WidgetType::Button, "button_1", 120.0f, 32.0f);
    firstButton.setProperty("onClick", "handleShared");
    auto secondButton = makeWidget(WidgetType::Button, "button_2", 120.0f, 32.0f);
    secondButton.setProperty("onRelease", "handleShared");

    document.root.appendChild(std::move(firstButton));
    document.root.appendChild(std::move(secondButton));
    document.refreshHierarchyMetadata();

    const ValidationReport report = ProjectValidator{}.validate(document, AppSettings{});

    CHECK(findMessage(report, "CALLBACK_NAME_INVALID") == nullptr);
    CHECK(findMessage(report, "CALLBACK_SIGNATURE_CONFLICT") == nullptr);
}
