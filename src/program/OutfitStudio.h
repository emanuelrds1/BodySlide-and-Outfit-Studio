/*
BodySlide and Outfit Studio
Copyright(C) 2017  Caliente & ousnius

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "OutfitProject.h"
#include "../ui/wxStateButton.h"
#include "../render/GLSurface.h"
#include "../components/TweakBrush.h"
#include "../components/Automorph.h"
#include "../utils/ConfigurationManager.h"

#include <wx/xrc/xmlres.h>
#include <wx/treectrl.h>
#include <wx/wizard.h>
#include <wx/filepicker.h>
#include <wx/grid.h>
#include <wx/spinctrl.h>
#include <wx/dataview.h>
#include <wx/splitter.h>
#include <wx/collpane.h>

enum TargetGame {
	FO3, FONV, SKYRIM, FO4, SKYRIMSE
};


class ShapeItemData : public wxTreeItemData  {
public:
	string shapeName;
	ShapeItemData(const string& inShapeName = "") {
		shapeName = inShapeName;
	}
};

class SegmentItemData : public wxTreeItemData  {
public:
	set<uint> tris;

	SegmentItemData(const set<uint>& inTriangles) {
		tris = inTriangles;
	}
};

class SubSegmentItemData : public wxTreeItemData  {
public:
	set<uint> tris;
	uint type;
	vector<float> extraData;

	SubSegmentItemData(const set<uint>& inTriangles, const uint& inType, const vector<float>& inExtraData = vector<float>()) {
		tris = inTriangles;
		type = inType;
		extraData = inExtraData;
	}
};

class PartitionItemData : public wxTreeItemData  {
public:
	vector<ushort> verts;
	vector<Triangle> tris;
	ushort type;

	PartitionItemData(const vector<ushort>& inVerts, const vector<Triangle>& inTris, const ushort& inType) {
		verts = inVerts;
		tris = inTris;
		type = inType;
	}
};

struct WeightCopyOptions {
	float proximityRadius;
	int maxResults;
};

struct ReferenceTemplate {
	string name;
	string sourceFile;
	string set;
	string shape;
};


class OutfitStudio;

class wxGLPanel : public wxGLCanvas {
public:
	wxGLPanel(wxWindow* parent, const wxSize& size, const wxGLAttributes& attribs);
	~wxGLPanel();

	void SetNotifyWindow(wxWindow* win);

	void AddMeshFromNif(NifFile* nif, const string& shapeName, bool buildNormals);

	void RenameShape(const string& shapeName, const string& newShapeName) {
		gls.RenameMesh(shapeName, newShapeName);
	}

	void SetMeshTextures(const string& shapeName, const vector<string>& textureFiles, const bool hasMatFile = false, const MaterialFile& matFile = MaterialFile());

	mesh* GetMesh(const string& shapeName) {
		return gls.GetMesh(shapeName);
	}

	void UpdateMeshVertices(const string& shapeName, vector<Vector3>* verts, bool updateBVH = true, bool recalcNormals = true, bool render = true, vector<Vector2>* uvs = nullptr);
	void RecalculateMeshBVH(const string& shapeName);

	void ShowShape(const string& shapeName, bool show = true);
	void SetActiveShapes(const vector<string>& shapeNames);
	void SetSelectedShape(const string& shapeName);

	TweakUndo* GetStrokeManager() {
		return strokeManager;
	}
	void SetStrokeManager(TweakUndo* manager) {
		if (!manager)
			strokeManager = &baseStrokes;
		else
			strokeManager = manager;
	}

	void SetActiveBrush(int brushID);
	TweakBrush* GetActiveBrush() {
		return activeBrush;
	}

	bool StartBrushStroke(const wxPoint& screenPos);
	void UpdateBrushStroke(const wxPoint& screenPos);
	void EndBrushStroke();

	bool StartTransform(const wxPoint& screenPos);
	void UpdateTransform(const wxPoint& screenPos);
	void EndTransform();

	bool SelectVertex(const wxPoint& screenPos);

	bool UndoStroke();
	bool RedoStroke();

	void ShowTransformTool(bool show = true, bool keepVisibility = false);
	void UpdateTransformTool();

	void ShowVertexEdit(bool show = true);
	
	bool GetEditMode() {
		return editMode;
	}
	void SetEditMode(bool on = true) {
		editMode = on;
	}

	bool GetVertexEdit() {
		return vertexEdit;
	}
	void SetVertexEdit(bool on = true) {
		vertexEdit = on;
		ShowVertexEdit(on);
	}

	bool GetTransformMode() {
		return transformMode;
	}
	void SetTransformMode(bool on = true) {
		transformMode = on;
		ShowTransformTool(on);
	}

	bool GetSegmentMode() {
		return segmentMode;
	}
	void SetSegmentMode(bool on = true) {
		segmentMode = on;
	}

	bool GetXMirror() {
		return bXMirror;
	}
	void SetXMirror(bool on = true) {
		bXMirror = on;
	}

	bool GetConnectedEdit() {
		return bConnectedEdit;
	}
	void SetConnectedEdit(bool on = true) {
		bConnectedEdit = on;
	}

	bool GetGlobalBrushCollision() {
		return bGlobalBrushCollision;
	}
	void SetGlobalBrushCollision(bool on = true) {
		bGlobalBrushCollision = on;
	}

	void SetShapeGhostMode(const string& shapeName, bool on = true) {
		mesh* m = gls.GetMesh(shapeName);
		if (!m) return;
		if (on)
			m->rendermode = RenderMode::LitWire;
		else
			m->rendermode = RenderMode::Normal;
	}
	void ToggleGhostMode() {
		for (auto &m : gls.GetActiveMeshes()) {
			if (m->rendermode == RenderMode::Normal)
				m->rendermode = RenderMode::LitWire;
			else if (m->rendermode == RenderMode::LitWire)
				m->rendermode = RenderMode::Normal;
		}
	}

	void ToggleNormalSeamSmoothMode() {
		for (auto &m : gls.GetActiveMeshes()) {
			if (m->smoothSeamNormals == true)
				m->smoothSeamNormals = false;
			else
				m->smoothSeamNormals = true;

			m->SmoothNormals();
		}
	}

	void RecalcNormals(const string& shape) {
		mesh* m = gls.GetMesh(shape);
		if (!m)
			return;

		m->SmoothNormals();
	}

	void ToggleAutoNormals() {
		if (bAutoNormals)
			bAutoNormals = false;
		else
			bAutoNormals = true;
	}

	float GetBrushSize() {
		return brushSize / 3.0f;
	}
	void SetBrushSize(float val) {
		val *= 3.0f;
		brushSize = val;
		gls.SetCursorSize(val);
	}

	float IncBrush() {
		gls.SetCursorSize(gls.GetCursorSize() + 0.010f);
		brushSize += 0.010f;
		LimitBrushSize();
		return brushSize;
	}
	float DecBrush() {
		gls.SetCursorSize(gls.GetCursorSize() - 0.010f);
		brushSize -= 0.010f;
		LimitBrushSize();
		return brushSize;
	}

	void LimitBrushSize() {
		if (brushSize < 0.000f) {
			gls.SetCursorSize(0.000f);
			brushSize = 0.000f;
		}
		else if (brushSize > 3.000f) {
			gls.SetCursorSize(3.000f);
			brushSize = 3.000f;
		}
	}

	float IncStr() {
		if (!activeBrush)
			return 0.0f;

		float str = activeBrush->getStrength();
		str += 0.010f;
		activeBrush->setStrength(str);
		return str;
	}
	float DecStr() {
		if (!activeBrush)
			return 0.0f;

		float str = activeBrush->getStrength();
		str -= 0.010f;
		activeBrush->setStrength(str);
		return str;
	}

	void ShowWireframe() {
		gls.ToggleWireframe();
	}

	void ToggleLighting() {
		gls.ToggleLighting();
	}

	void ToggleTextures() {
		gls.ToggleTextures();
	}

	void SetMaskVisible(bool bVisible = true) {
		gls.SetMaskVisible(bVisible);
	}

	void SetWeightVisible(bool bVisible = true) {
		gls.SetWeightColors(bVisible);
	}

	void SetSegmentsVisible(bool bVisible = true) {
		gls.SetSegmentColors(bVisible);
	}

	void ClearMask() {
		for (auto &m : gls.GetActiveMeshes())
			m->ColorChannelFill(0, 0.0f);
	}
	
	void ClearColorChannels() {
		for (auto &m : gls.GetActiveMeshes())
			m->ColorFill(Vector3(0.0f, 0.0f, 0.0f));
	}

	void GetActiveMask(unordered_map<ushort, float>& mask) {
		if (gls.GetActiveMeshes().empty())
			return;

		mesh* m = gls.GetActiveMeshes().back();

		for (int i = 0; i < m->nVerts; i++) {
			if (m->vcolors[i].x != 0.0f)
				mask[i] = m->vcolors[i].x;
		}
	}

	void GetActiveUnmasked(unordered_map<ushort, float>& mask) {
		if (gls.GetActiveMeshes().empty())
			return;

		mesh* m = gls.GetActiveMeshes().back();

		for (int i = 0; i < m->nVerts; i++)
			if (m->vcolors[i].x == 0.0f)
				mask[i] = m->vcolors[i].x;
	}

	void GetShapeMask(unordered_map<ushort, float>& mask, const string& shapeName) {
		mesh* m = gls.GetMesh(shapeName);
		if (!m)
			return;

		for (int i = 0; i < m->nVerts; i++) {
			if (m->vcolors[i].x != 0.0f)
				mask[i] = m->vcolors[i].x;
		}
	}

	void GetShapeUnmasked(unordered_map<ushort, float>& mask, const string& shapeName) {
		mesh* m = gls.GetMesh(shapeName);
		if (!m)
			return;

		for (int i = 0; i < m->nVerts; i++)
			if (m->vcolors[i].x == 0.0f)
				mask[i] = m->vcolors[i].x;
	}

	void InvertMaskTris(unordered_map<ushort, float>& mask, const string& shapeName) {
		mesh* m = gls.GetMesh(shapeName);
		if (!m)
			return;

		unordered_map<ushort, float> triMask;
		for (int t = 0; t < m->nTris; t++) {
			if (mask.find(m->tris[t].p1) != mask.end() ||
				mask.find(m->tris[t].p2) != mask.end() ||
				mask.find(m->tris[t].p3) != mask.end()) {
				triMask.emplace(m->tris[t].p1, 1.0f);
				triMask.emplace(m->tris[t].p2, 1.0f);
				triMask.emplace(m->tris[t].p3, 1.0f);
			}
		}

		unordered_map<ushort, float> invertMask;
		for (int t = 0; t < m->nTris; t++) {
			if (triMask.find(m->tris[t].p1) == triMask.end())
				invertMask.emplace(m->tris[t].p1, 1.0f);
			if (triMask.find(m->tris[t].p2) == triMask.end())
				invertMask.emplace(m->tris[t].p2, 1.0f);
			if (triMask.find(m->tris[t].p3) == triMask.end())
				invertMask.emplace(m->tris[t].p3, 1.0f);
		}

		mask = move(invertMask);
	}

	void InvertMask() {
		for (auto &m : gls.GetActiveMeshes()) {
			for (int i = 0; i < m->nVerts; i++)
				m->vcolors[i].x = 1.0f - m->vcolors[i].x;

			m->QueueUpdate(mesh::UpdateType::VertexColors);
		}
	}

	void DeleteMesh(const string& shape) {
		gls.DeleteMesh(shape);
	}

	void DestroyOverlays() {
		gls.DeleteOverlays();
	}

	void Cleanup() {
		XMoveMesh = nullptr;
		YMoveMesh = nullptr;
		ZMoveMesh = nullptr;
		XRotateMesh = nullptr;
		YRotateMesh = nullptr;
		ZRotateMesh = nullptr;
		XScaleMesh = nullptr;
		YScaleMesh = nullptr;
		ZScaleMesh = nullptr;
		ScaleUniformMesh = nullptr;
		gls.Cleanup();
	}

	void SetView(const char type) {
		gls.SetView(type);

		if (transformMode)
			ShowTransformTool();
		else
			Render();
	}

	void SetPerspective(const bool enabled) {
		gls.SetPerspective(enabled);
		gls.RenderOneFrame();
	}

	void SetFieldOfView(const int fieldOfView) {
		gls.SetFieldOfView(fieldOfView);
		gls.RenderOneFrame();
	}

	void UpdateLights(const int ambient, const int frontal, const int directional0, const int directional1, const int directional2,
		const Vector3 directional0Dir = Vector3(), const Vector3 directional1Dir = Vector3(), const Vector3 directonal2Dir = Vector3())
	{
		gls.UpdateLights(ambient, frontal, directional0, directional1, directional2, directional0Dir, directional1Dir, directonal2Dir);
		gls.RenderOneFrame();
	}

	void Render() {
		gls.RenderOneFrame();
	}


private:
	void OnShown();
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

	void OnMouseWheel(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);

	void OnMiddleDown(wxMouseEvent& event);
	void OnMiddleUp(wxMouseEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);

	void OnRightDown(wxMouseEvent& event);
	void OnRightUp(wxMouseEvent& event);

	void OnKeys(wxKeyEvent& event);
	void OnIdle(wxIdleEvent& event);

	void OnCaptureLost(wxMouseCaptureLostEvent& event);

	GLSurface gls;
	wxGLContext* context;

	bool rbuttonDown;
	bool lbuttonDown;
	bool mbuttonDown;
	bool isLDragging;
	bool isMDragging;
	bool isRDragging;
	bool firstPaint{true};

	int lastX;
	int lastY;

	set<int> BVHUpdateQueue;

	OutfitStudio* os;

	float brushSize;

	bool editMode;
	bool transformMode;
	bool vertexEdit;
	bool segmentMode;

	bool bMaskPaint;
	bool bWeightPaint;
	bool isPainting;
	bool isTransforming;
	bool isSelecting;
	bool bAutoNormals;
	bool bXMirror;
	bool bConnectedEdit;
	bool bGlobalBrushCollision;

	TweakBrush* activeBrush;
	TweakBrush* savedBrush;
	TweakBrush standardBrush;
	TB_Deflate deflateBrush;
	TB_Move moveBrush;
	TB_Smooth smoothBrush;
	TB_Mask maskBrush;
	TB_Unmask UnMaskBrush;
	TB_Weight weightBrush;
	TB_Unweight unweightBrush;
	TB_SmoothWeight smoothWeightBrush;
	TB_XForm translateBrush;

	TweakStroke* activeStroke;
	TweakUndo* strokeManager;
	TweakUndo baseStrokes;

	mesh* XMoveMesh = nullptr;
	mesh* YMoveMesh = nullptr;
	mesh* ZMoveMesh = nullptr;
	mesh* XRotateMesh = nullptr;
	mesh* YRotateMesh = nullptr;
	mesh* ZRotateMesh = nullptr;
	mesh* XScaleMesh = nullptr;
	mesh* YScaleMesh = nullptr;
	mesh* ZScaleMesh = nullptr;
	mesh* ScaleUniformMesh = nullptr;
	Vector3 xformCenter;		// Transform center for transform brushes (rotate, specifically cares about this)
	float lastCenterDistance;

	wxDECLARE_EVENT_TABLE();
};

class OutfitProject;

class OutfitStudio : public wxFrame {
public:
	OutfitStudio(const wxPoint& pos, const wxSize& size, ConfigurationManager& inConfig);
	~OutfitStudio();

	TargetGame targetGame;
	wxGLPanel* glView = nullptr;
	OutfitProject* project = nullptr;
	ShapeItemData* activeItem = nullptr;
	string activeSlider;
	bool bEditSlider;

	wxTreeCtrl* outfitShapes;
	wxTreeCtrl* outfitBones;
	wxTreeCtrl* segmentTree;
	wxTreeCtrl* partitionTree;
	wxPanel* lightSettings;
	wxSlider* boneScale;
	wxScrolledWindow* sliderScroll;
	wxStatusBar* statusBar;
	wxTreeItemId shapesRoot;
	wxTreeItemId outfitRoot;
	wxTreeItemId bonesRoot;
	wxTreeItemId segmentRoot;
	wxTreeItemId partitionRoot;
	wxImageList* visStateImages;

	ConfigurationManager& appConfig;

	class SliderDisplay {
	public:
		bool hilite;
		wxPanel* sliderPane;
		wxBoxSizer* paneSz;

		int sliderNameCheckID;
		int sliderID;

		wxBitmapButton* btnSliderEdit;
		wxButton* btnMinus;
		wxButton* btnPlus;
		wxCheckBox* sliderNameCheck;
		wxStaticText* sliderName;
		wxSlider* slider;
		wxTextCtrl* sliderReadout;

		TweakUndo sliderStrokes;			// This probably shouldn't be here, but it's a convenient location to store undo info.
	};

	map<string, SliderDisplay*> sliderDisplays;

	void CreateSetSliders();

	string NewSlider(const string& suggestedName = "", bool skipPrompt = false);

	void SetSliderValue(int index, int val);
	void SetSliderValue(const string& name, int val);

	void ApplySliders(bool recalcBVH = true);

	void ShowSliderEffect(int slider, bool show = true);
	void ShowSliderEffect(const string& sliderName, bool show = true);

	void SelectShape(const string& shapeName);
	vector<string> GetShapeList();

	void UpdateShapeSource(const string& shapeName);

	void ActiveShapesUpdated(TweakStroke* refStroke, bool bIsUndo = false, bool setWeights = true);
	void UpdateActiveShapeUI();

	void ShowSegment(const wxTreeItemId& item = nullptr, bool updateFromMask = false);
	void UpdateSegmentNames();

	void ShowPartition(const wxTreeItemId& item = nullptr, bool updateFromMask = false);
	void UpdatePartitionNames();

	void AnimationGUIFromProj();
	void RefreshGUIFromProj();

	vector<ShapeItemData*>& GetSelectedItems();
	string GetActiveBone();

	bool NotifyStrokeStarting();

	void EnterSliderEdit(const string& sliderName);
	void ExitSliderEdit();
	void MenuEnterSliderEdit();
	void MenuExitSliderEdit();

	void ToggleBrushPane(bool forceCollapse = false) {
		wxCollapsiblePane* brushPane = (wxCollapsiblePane*)FindWindowByName("brushPane");
		if (!brushPane)
			return;

		brushPane->Collapse(!brushPane->IsCollapsed() || forceCollapse);

		wxWindow* leftPanel = FindWindowByName("leftSplitPanel");
		if (leftPanel)
			leftPanel->Layout();
	}

	void UpdateBrushPane() {
		TweakBrush* brush = glView->GetActiveBrush();
		if (!brush)
			return;

		wxCollapsiblePane* parent = (wxCollapsiblePane*)FindWindowByName("brushPane");
		if (!parent)
			return;

		XRCCTRL(*parent, "brushSize", wxSlider)->SetValue(glView->GetBrushSize() * 1000.0f);
		wxStaticText* valSize = (wxStaticText*)XRCCTRL(*parent, "valSize", wxStaticText);
		wxString valSizeStr = wxString::Format("%0.3f", glView->GetBrushSize());
		valSize->SetLabel(valSizeStr);

		XRCCTRL(*parent, "brushStr", wxSlider)->SetValue(brush->getStrength() * 1000.0f);
		wxStaticText* valStr = (wxStaticText*)XRCCTRL(*parent, "valStr", wxStaticText);
		wxString valStrengthStr = wxString::Format("%0.3f", brush->getStrength());
		valStr->SetLabel(valStrengthStr);

		XRCCTRL(*parent, "brushFocus", wxSlider)->SetValue(brush->getFocus() * 1000.0f);
		wxStaticText* valFocus = (wxStaticText*)XRCCTRL(*parent, "valFocus", wxStaticText);
		wxString valFocusStr = wxString::Format("%0.3f", brush->getFocus());
		valFocus->SetLabel(valFocusStr);

		XRCCTRL(*parent, "brushSpace", wxSlider)->SetValue(brush->getSpacing() * 1000.0f);
		wxStaticText* valSpace = (wxStaticText*)XRCCTRL(*parent, "valSpace", wxStaticText);
		wxString valSpacingStr = wxString::Format("%0.3f", brush->getSpacing());
		valSpace->SetLabel(valSpacingStr);
	}

	void CheckBrushBounds() {
		TweakBrush* brush = glView->GetActiveBrush();
		if (!brush)
			return;

		float size = glView->GetBrushSize();
		float strength = brush->getStrength();
		//float focus = brush->getFocus();
		//float spacing = brush->getSpacing();

		if (size >= 1.0f)
			GetMenuBar()->Enable(XRCID("btnIncreaseSize"), false);
		else
			GetMenuBar()->Enable(XRCID("btnIncreaseSize"), true);

		if (size <= 0.0f)
			GetMenuBar()->Enable(XRCID("btnDecreaseSize"), false);
		else
			GetMenuBar()->Enable(XRCID("btnDecreaseSize"), true);

		if (strength >= 1.0f)
			GetMenuBar()->Enable(XRCID("btnIncreaseStr"), false);
		else
			GetMenuBar()->Enable(XRCID("btnIncreaseStr"), true);

		if (strength <= 0.0f)
			GetMenuBar()->Enable(XRCID("btnDecreaseStr"), false);
		else
			GetMenuBar()->Enable(XRCID("btnDecreaseStr"), true);
	}

	wxGauge* progressBar = nullptr;
	vector<pair<int, int>> progressStack;
	int progressVal = 0;

	void StartProgress(const wxString& msg = "") {
		if (progressStack.empty()) {
			progressVal = 0;
			progressStack.emplace_back(0, 10000);

			wxRect rect;
			statusBar->GetFieldRect(1, rect);

			wxBeginBusyCursor();
			progressBar = new wxGauge(statusBar, wxID_ANY, 10000, rect.GetPosition(), rect.GetSize());

			if (msg.IsEmpty())
				statusBar->SetStatusText(_("Starting..."));
			else
				statusBar->SetStatusText(msg);
		}
	}

	void StartSubProgress(int min, int max) {
		int range = progressStack.back().second - progressStack.front().first;
		float mindiv = min / 100.0f;
		float maxdiv = max / 100.0f;
		int minoff = mindiv * range + 1;
		int maxoff = maxdiv * range + 1;
		progressStack.emplace_back(progressStack.front().first + minoff, progressStack.front().first + maxoff);
	}

	void EndProgress(const wxString& msg = "") {
		if (progressStack.empty())
			return;

		progressBar->SetValue(progressStack.back().second);
		progressStack.pop_back();

		if (progressStack.empty()) {
			if (msg.IsEmpty())
				statusBar->SetStatusText(_("Ready!"));
			else
				statusBar->SetStatusText(msg);

			delete progressBar;
			progressBar = nullptr;
			wxEndBusyCursor();
		}
	}

	void UpdateProgress(int val, const wxString& msg = "") {
		if (progressStack.empty())
			return;

		int range = progressStack.back().second - progressStack.back().first;
		float div = val / 100.0f;
		int offset = range * div + 1;

		progressVal = progressStack.back().first + offset;
		if (progressVal > 10000)
			progressVal = 10000;

		statusBar->SetStatusText(msg);
		progressBar->SetValue(progressVal);
	}

private:
	bool previousMirror;
	Vector3 previewMove;
	Vector3 previewScale;
	Vector3 previewRotation;
	unordered_map<string, unordered_map<ushort, Vector3>> previewDiff;

	vector<ShapeItemData*> selectedItems;
	string activeBone;
	wxTreeItemId activeSegment;
	wxTreeItemId activePartition;

	vector<ReferenceTemplate> refTemplates;

	void createSliderGUI(const string& name, int id, wxScrolledWindow* wnd, wxSizer* rootSz);
	void HighlightSlider(const string& name);
	void ZeroSliders();

	void UpdateReferenceTemplates();

	void ClearProject();
	void RenameProject(const string& projectName);

	void WorkingGUIFromProj();

	bool ShowWeightCopy(WeightCopyOptions& options);
	void ReselectBone();

	void OnSashPosChanged(wxSplitterEvent& event);
	void OnMoveWindow(wxMoveEvent& event);
	void OnSetSize(wxSizeEvent& event);

	void OnExit(wxCommandEvent& event);
	void OnNewProject(wxCommandEvent& event);
	void OnLoadProject(wxCommandEvent &event);
	void OnLoadReference(wxCommandEvent &event);
	void OnLoadOutfit(wxCommandEvent& event);
	void OnUnloadProject(wxCommandEvent &event);

	void OnLoadOutfitFP_File(wxFileDirPickerEvent& event);
	void OnLoadOutfitFP_Texture(wxFileDirPickerEvent& event);

	void OnSetBaseShape(wxCommandEvent &event);
	void OnMakeConvRef(wxCommandEvent& event);

	void OnImportNIF(wxCommandEvent &event);
	void OnExportNIF(wxCommandEvent &event);
	void OnExportNIFWithRef(wxCommandEvent &event);
	void OnExportShapeNIF(wxCommandEvent& event);

	void OnImportOBJ(wxCommandEvent& event);
	void OnExportOBJ(wxCommandEvent& event);
	void OnExportShapeOBJ(wxCommandEvent& event);

	void OnImportFBX(wxCommandEvent& event);
	void OnExportFBX(wxCommandEvent& event);
	void OnExportShapeFBX(wxCommandEvent& event);

	void OnImportPhysicsData(wxCommandEvent &event);
	void OnExportPhysicsData(wxCommandEvent &event);

	void OnSSSNameCopy(wxCommandEvent& event);
	void OnSSSGenWeightsTrue(wxCommandEvent& event);
	void OnSSSGenWeightsFalse(wxCommandEvent& event);
	void OnSaveSliderSet(wxCommandEvent &event);
	void OnSaveSliderSetAs(wxCommandEvent &event);

	void OnBrushPane(wxCollapsiblePaneEvent &event);

	void OnSlider(wxScrollEvent& event);
	void OnClickSliderButton(wxCommandEvent &event);
	void OnReadoutChange(wxCommandEvent& event);
	void OnCheckBox(wxCommandEvent& event);

	void OnTabButtonClick(wxCommandEvent& event);
	void OnFixedWeight(wxCommandEvent& event);
	void OnSelectSliders(wxCommandEvent& event);

	void OnShapeVisToggle(wxTreeEvent& event);
	void OnShapeSelect(wxTreeEvent& event);
	void OnShapeActivated(wxTreeEvent& event);
	void OnShapeContext(wxTreeEvent& event);
	void OnShapeDrag(wxTreeEvent& event);
	void OnShapeDrop(wxTreeEvent& event);
	void OnCheckTreeSel(wxTreeEvent& event);

	void OnBoneSelect(wxTreeEvent& event);
	void OnBoneContext(wxTreeEvent& event);
	void OnBoneTreeContext(wxCommandEvent& event);

	void OnSegmentSelect(wxTreeEvent& event);
	void OnSegmentContext(wxTreeEvent& event);
	void OnSegmentTreeContext(wxCommandEvent& event);
	void OnAddSegment(wxCommandEvent& event);
	void OnAddSubSegment(wxCommandEvent& event);
	void OnDeleteSegment(wxCommandEvent& event);
	void OnDeleteSubSegment(wxCommandEvent& event);
	void OnSegmentTypeChanged(wxCommandEvent& event);
	void OnSegmentApply(wxCommandEvent& event);
	void OnSegmentReset(wxCommandEvent& event);

	void OnPartitionSelect(wxTreeEvent& event);
	void OnPartitionContext(wxTreeEvent& event);
	void OnPartitionTreeContext(wxCommandEvent& event);
	void OnAddPartition(wxCommandEvent& event);
	void OnDeletePartition(wxCommandEvent& event);
	void OnPartitionTypeChanged(wxCommandEvent& event);
	void OnPartitionApply(wxCommandEvent& event);
	void OnPartitionReset(wxCommandEvent& event);

	void CreateSegmentTree(const string& shapeName = "");
	void CreatePartitionTree(const string& shapeName = "");

	void OnSelectTool(wxCommandEvent& event);

	void OnSetView(wxCommandEvent& event);
	void OnTogglePerspective(wxCommandEvent& event);
	void OnFieldOfViewSlider(wxCommandEvent& event);
	void OnUpdateLights(wxCommandEvent& event);
	void OnResetLights(wxCommandEvent& event);

	void OnLoadPreset(wxCommandEvent& event);
	void OnSavePreset(wxCommandEvent& event);
	void OnSliderConform(wxCommandEvent& event);
	void OnSliderConformAll(wxCommandEvent& event);
	void OnSliderImportBSD(wxCommandEvent& event);
	void OnSliderImportOBJ(wxCommandEvent& event);
	void OnSliderImportOSD(wxCommandEvent& event);
	void OnSliderImportTRI(wxCommandEvent& event);
	void OnSliderImportFBX(wxCommandEvent& event);
	void OnSliderExportBSD(wxCommandEvent& event);
	void OnSliderExportOBJ(wxCommandEvent& event);
	void OnSliderExportOSD(wxCommandEvent& event);
	void OnSliderExportTRI(wxCommandEvent& event);

	void OnNewSlider(wxCommandEvent& event);
	void OnNewZapSlider(wxCommandEvent& event);
	void OnNewCombinedSlider(wxCommandEvent& event);
	void OnSliderNegate(wxCommandEvent& event);
	void OnMaskAffected(wxCommandEvent& event);
	void OnClearSlider(wxCommandEvent& event);
	void OnDeleteSlider(wxCommandEvent& event);
	void OnSliderProperties(wxCommandEvent& event);

	void OnInvertUV(wxCommandEvent& event);

	void OnEnterClose(wxKeyEvent& event);
	
	void OnMoveShape(wxCommandEvent& event);
	void OnMoveShapeOldOffset(wxCommandEvent& event);
	void OnMoveShapeSlider(wxCommandEvent& event);
	void OnMoveShapeText(wxCommandEvent& event);
	void PreviewMove(const Vector3& changed);

	void OnScaleShape(wxCommandEvent& event);
	void OnScaleShapeSlider(wxCommandEvent& event);
	void OnScaleShapeText(wxCommandEvent& event);
	void PreviewScale(const Vector3& scale);

	void OnRotateShape(wxCommandEvent& event);
	void OnRotateShapeSlider(wxCommandEvent& event);
	void OnRotateShapeText(wxCommandEvent& event);
	void PreviewRotation(const Vector3& changed);

	void OnRenameShape(wxCommandEvent& event);
	void OnSetReference(wxCommandEvent& event);
	void OnDeleteVerts(wxCommandEvent& event);
	void OnSeparateVerts(wxCommandEvent& event);
	void OnDupeShape(wxCommandEvent& event);
	void OnDeleteShape(wxCommandEvent& event);
	void OnAddBone(wxCommandEvent& event);
	void OnAddCustomBone(wxCommandEvent& event);
	void OnDeleteBone(wxCommandEvent& event);
	void OnDeleteBoneFromSelected(wxCommandEvent& event);
	void OnCopyBoneWeight(wxCommandEvent& event);
	void OnCopySelectedWeight(wxCommandEvent& event);
	void OnTransferSelectedWeight(wxCommandEvent& event);
	void OnMaskWeighted(wxCommandEvent& event);
	void OnShapeProperties(wxCommandEvent& event);

	void OnNPWizChangeSliderSetFile(wxFileDirPickerEvent& event);
	void OnNPWizChangeSetNameChoice(wxCommandEvent& event);

	void OnBrushSettingsSlider(wxScrollEvent& event);

	void OnXMirror(wxCommandEvent& event) {
		bool enabled = event.IsChecked();
		GetMenuBar()->Check(event.GetId(), enabled);
		GetToolBar()->ToggleTool(event.GetId(), enabled);
		glView->SetXMirror(enabled);
	}

	void OnConnectedOnly(wxCommandEvent& event) {
		bool enabled = event.IsChecked();
		GetMenuBar()->Check(event.GetId(), enabled);
		GetToolBar()->ToggleTool(event.GetId(), enabled);
		glView->SetConnectedEdit(enabled);
	}

	void OnGlobalBrushCollision(wxCommandEvent& event) {
		bool enabled = event.IsChecked();
		GetMenuBar()->Check(event.GetId(), enabled);
		GetToolBar()->ToggleTool(event.GetId(), enabled);
		glView->SetGlobalBrushCollision(enabled);
	}

	void OnUndo(wxCommandEvent& WXUNUSED(event)) {
		if (glView->GetSegmentMode())
			return;

		glView->UndoStroke();
	}
	void OnRedo(wxCommandEvent& WXUNUSED(event)) {
		if (glView->GetSegmentMode())
			return;

		glView->RedoStroke();
	}

	void OnRecalcNormals(wxCommandEvent& WXUNUSED(event)) {
		glView->RecalcNormals(activeItem->shapeName);
		glView->Render();
	}

	void OnAutoNormals(wxCommandEvent& WXUNUSED(event)) {
		glView->ToggleAutoNormals();
		glView->Render();
	}

	void OnSmoothNormalSeams(wxCommandEvent& WXUNUSED(event)) {
		glView->ToggleNormalSeamSmoothMode();
		glView->Render();
	}

	void OnGhostMesh(wxCommandEvent& WXUNUSED(event)) {
		glView->ToggleGhostMode();
		glView->Render();
	}

	void OnShowWireframe(wxCommandEvent& WXUNUSED(event)) {
		glView->ShowWireframe();
		glView->Render();
	}

	void OnEnableLighting(wxCommandEvent& WXUNUSED(event)) {
		glView->ToggleLighting();
		glView->Render();
	}

	void OnEnableTextures(wxCommandEvent& WXUNUSED(event)) {
		glView->ToggleTextures();
		glView->Render();
	}

	void OnShowMask(wxCommandEvent& event) {
		glView->SetMaskVisible(event.IsChecked());
		glView->Render();
	}

	void OnIncBrush(wxCommandEvent& WXUNUSED(event)) {
		if (glView->GetActiveBrush() && glView->GetBrushSize() < 1.0f) {
			float v = glView->IncBrush() / 3.0f;
			if (statusBar)
				statusBar->SetStatusText(wxString::Format("Rad: %f", v), 2);

			CheckBrushBounds();
			UpdateBrushPane();
		}
	}
	void OnDecBrush(wxCommandEvent& WXUNUSED(event)) {
		if (glView->GetActiveBrush() && glView->GetBrushSize() > 0.0f) {
			float v = glView->DecBrush() / 3.0f;
			if (statusBar)
				statusBar->SetStatusText(wxString::Format("Rad: %f", v), 2);

			CheckBrushBounds();
			UpdateBrushPane();
		}
	}
	void OnIncStr(wxCommandEvent& WXUNUSED(event)) {
		if (glView->GetActiveBrush() && glView->GetActiveBrush()->getStrength() < 1.0f) {
			float v = glView->IncStr();
			if (statusBar)
				statusBar->SetStatusText(wxString::Format("Str: %f", v), 2);

			CheckBrushBounds();
			UpdateBrushPane();
		}
	}
	void OnDecStr(wxCommandEvent& WXUNUSED(event)) {
		if (glView->GetActiveBrush() && glView->GetActiveBrush()->getStrength() > 0.0f) {
			float v = glView->DecStr();
			if (statusBar)
				statusBar->SetStatusText(wxString::Format("Str: %f", v), 2);

			CheckBrushBounds();
			UpdateBrushPane();
		}
	}

	void OnClearMask(wxCommandEvent& WXUNUSED(event)) {
		if (!activeItem)
			return;

		glView->ClearMask();

		if (glView->GetTransformMode())
			glView->ShowTransformTool();
		else
			glView->Render();
	}

	void OnInvertMask(wxCommandEvent& WXUNUSED(event)) {
		if (!activeItem)
			return;

		glView->InvertMask();

		if (glView->GetTransformMode())
			glView->ShowTransformTool();
		else
			glView->Render();
	}

	wxDECLARE_EVENT_TABLE();
};

class DnDFile : public wxFileDropTarget {
public:
	DnDFile(OutfitStudio *pOwner = nullptr) { owner = pOwner; }

	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& fileNames);

private:
	OutfitStudio *owner;
};

class DnDSliderFile : public wxFileDropTarget {
public:
	DnDSliderFile(OutfitStudio *pOwner = nullptr) { owner = pOwner; }

	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& fileNames);
	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);

private:
	OutfitStudio *owner;
	wxDragResult lastResult;
	string targetSlider;
};
