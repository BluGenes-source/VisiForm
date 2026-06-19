#pragma once

#pragma once

#include <string_view>

namespace visiform::commands::ids {

inline constexpr std::string_view kFileNew = "file.new";
inline constexpr std::string_view kFileOpen = "file.open";
inline constexpr std::string_view kFileSave = "file.save";
inline constexpr std::string_view kFileSaveAs = "file.saveAs";
inline constexpr std::string_view kFileExport = "file.export";

inline constexpr std::string_view kEditUndo = "edit.undo";
inline constexpr std::string_view kEditRedo = "edit.redo";
inline constexpr std::string_view kEditCopy = "edit.copy";
inline constexpr std::string_view kEditPaste = "edit.paste";
inline constexpr std::string_view kEditDelete = "edit.delete";
inline constexpr std::string_view kEditDuplicate = "edit.duplicate";

inline constexpr std::string_view kViewGrid = "view.grid";
inline constexpr std::string_view kViewSnap = "view.snap";
inline constexpr std::string_view kViewGuides = "view.guides";
inline constexpr std::string_view kViewMultiSelect = "view.multiSelect";
inline constexpr std::string_view kViewPreview = "view.preview";
inline constexpr std::string_view kViewZoomIn = "view.zoomIn";
inline constexpr std::string_view kViewZoomOut = "view.zoomOut";
inline constexpr std::string_view kViewZoomReset = "view.zoomReset";
inline constexpr std::string_view kViewZoomFit = "view.zoomFit";

inline constexpr std::string_view kLayoutFitText = "layout.fitText";
inline constexpr std::string_view kLayoutFitWidthToParent = "layout.fitWidthToParent";
inline constexpr std::string_view kLayoutFitHeightToParent = "layout.fitHeightToParent";
inline constexpr std::string_view kLayoutAlignLeft = "layout.alignLeft";
inline constexpr std::string_view kLayoutAlignTop = "layout.alignTop";
inline constexpr std::string_view kLayoutAlignRight = "layout.alignRight";
inline constexpr std::string_view kLayoutAlignBottom = "layout.alignBottom";
inline constexpr std::string_view kLayoutCenterHorizontal = "layout.centerHorizontal";
inline constexpr std::string_view kLayoutCenterVertical = "layout.centerVertical";
inline constexpr std::string_view kLayoutSameWidth = "layout.sameWidth";
inline constexpr std::string_view kLayoutSameWidthSmallest = "layout.sameWidthSmallest";
inline constexpr std::string_view kLayoutSameWidthLargest = "layout.sameWidthLargest";
inline constexpr std::string_view kLayoutSameHeight = "layout.sameHeight";
inline constexpr std::string_view kLayoutSameHeightSmallest = "layout.sameHeightSmallest";
inline constexpr std::string_view kLayoutSameHeightLargest = "layout.sameHeightLargest";
inline constexpr std::string_view kLayoutDistributeHorizontal = "layout.distributeHorizontal";
inline constexpr std::string_view kLayoutDistributeVertical = "layout.distributeVertical";
inline constexpr std::string_view kLayoutBringForward = "layout.bringForward";
inline constexpr std::string_view kLayoutSendBackward = "layout.sendBackward";

inline constexpr std::string_view kProjectValidate = "project.validate";
inline constexpr std::string_view kProjectSettings = "project.settings";
inline constexpr std::string_view kProjectEditLookAndFeel = "project.editLookAndFeel";
inline constexpr std::string_view kProjectResources = "project.resources";
inline constexpr std::string_view kProjectKeyboardShortcuts = "project.keyboardShortcuts";

inline constexpr std::string_view kHelpAbout = "help.about";
inline constexpr std::string_view kHelpGeneratedCodeGuide = "help.generatedCodeGuide";
inline constexpr std::string_view kViewValidationReport = "view.validationReport";
inline constexpr std::string_view kFileOpenSample = "file.openSample";
inline constexpr std::string_view kProjectExportDependencies = "project.exportDependencies";

} // namespace visiform::commands::ids
