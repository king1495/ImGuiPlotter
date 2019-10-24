#pragma once
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>

#include <ImGui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui_internal.h>

#include "ImPlot.hpp"

template<typename T>
inline std::string ntos(T _val, int _precision) {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(_precision) << _val;
	std::string result = ss.str();
	return result;
}
template<typename T>
inline std::wstring ntow(T _val, int _precision) {
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(_precision) << _val;
	std::wstring result = ss.str();
	return result;
}

inline ImVec2 CalcVerticalTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f)
{
	const ImVec2 temp = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
	return ImVec2(temp.y, temp.x);
}

inline void AddRotateText(const ImVec2& pos, const float& deg, ImU32 col, const char* text_begin, const char* text_end = NULL)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	if (text_end == NULL)
		text_end = text_begin + strlen(text_begin);
	if (text_begin == text_end)
		return;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImVector<ImDrawVert>& vertexBuffer = window->DrawList->VtxBuffer;
	int start_ind = vertexBuffer.Size;

	window->DrawList->AddText(pos, col, text_begin, text_end);

	float s = sin(radians(deg)), c = cos(radians(deg));
	ImVec2 center = ImRotate(pos, c, s) - pos;

	for (int i = start_ind; i < vertexBuffer.Size; i++)
		vertexBuffer[i].pos = ImRotate(vertexBuffer[i].pos, c, s) - center;
	//window->DrawList->UpdateClipRect();
}

template<typename T>
class ImAxes
{
public:
	ImAxes()
		: axesCoordType(ImPlotCoordType_Cartesian)
		, axesColor(ImColor(255, 255, 255)), axesWidth(1.5f)
		, gridColor(ImColor(170, 170, 170)), gridWidth(0.5f)
		, xlabelSize(ImGui::GetFontSize()), ylabelSize(ImGui::GetFontSize())
		, xlabel(L""), ylabel(L"")
		, xGridOn(false), yGridOn(false)
		, xPrecision(0), yPrecision(0)
		, xlim(0, 1), ylim(0, 1)
		, xtickNum(9), ytickNum(9)
	{};
	virtual ~ImAxes() {};

	void AddImPlot(std::shared_ptr<ImPlot<T>> _plot) {
		vPlots.emplace_back(_plot);
	}

	void ClearImPlot() {
		vPlots.clear();
		vPlots.shrink_to_fit();
	}

	void Render();

	template<typename T>
	friend class ImGuiPlotter;

	template<typename T>
	friend class ImPlot;

private:
	std::vector<std::shared_ptr<ImPlot<T>>> vPlots;

public:
	ImPlotCoordType_ axesCoordType;

	std::wstring xlabel;
	float xlabelSize;

	std::wstring ylabel;
	float ylabelSize;

	ImColor axesColor;
	float axesWidth;

	ImColor gridColor;
	float gridWidth;

	ImVec2 xlim;
	bool xGridOn;
	int xtickNum;

	ImVec2 ylim;
	bool yGridOn;
	int ytickNum;

	int xPrecision;
	int yPrecision;
private:
	ImRect axes_bb;
	ImRect canvas_bb;
};

template<typename T>
inline void ImAxes<T>::Render()
{
	using namespace ImGui;
	if (vPlots.empty()) return;
	ImGuiWindow* window = GetCurrentWindow();
	char ylchar[256], xlchar[256];

	if (ytickNum < 2) ytickNum = 2;
	if (xtickNum < 2) xtickNum = 2;
	if (yPrecision < 0) yPrecision = 0;
	if (xPrecision < 0) xPrecision = 0;

	ImVec2 yval_min_tsize = CalcTextSize(ntos(ylim.x, yPrecision).c_str());
	ImVec2 yval_max_tsize = CalcTextSize(ntos(ylim.y, yPrecision).c_str());
	ImVec2 xval_min_tsize = CalcTextSize(ntos(xlim.x, xPrecision).c_str());
	ImVec2 xval_max_tsize = CalcTextSize(ntos(xlim.y, xPrecision).c_str());

	ImVec2 xval_tsize = ImMax(xval_min_tsize, xval_max_tsize);
	ImVec2 yval_tsize = ImMax(yval_min_tsize, yval_max_tsize);
	ImVec2 xlabel_tsize = { 0.f,0.f };
	ImVec2 ylabel_tsize = { 0.f,0.f };

	if (ylabel != L"") {
		WideCharToMultiByte(CP_UTF8, 0, ylabel.c_str(), -1, ylchar, IM_ARRAYSIZE(ylchar), NULL, NULL);
		ylabel_tsize = CalcVerticalTextSize(ylchar);
	}

	if (xlabel != L"") {
		WideCharToMultiByte(CP_UTF8, 0, xlabel.c_str(), -1, xlchar, IM_ARRAYSIZE(xlchar), NULL, NULL);
		xlabel_tsize = CalcTextSize(xlchar);
	}

	ImRect data_bb;
	switch (axesCoordType) {
	case ImPlotCoordType_Polar:
	{
		ImVec2 tempSize = CalcTextSize(ntos(180.f, yPrecision).c_str());
		canvas_bb = ImRect(axes_bb.Min + tempSize * 1.f, axes_bb.Max - tempSize * 1.f);
		ImVec2 center = canvas_bb.GetCenter();
		float maxRadius = 0.5f * ((canvas_bb.GetWidth() > canvas_bb.GetHeight()) ? canvas_bb.GetHeight() : canvas_bb.GetWidth());
		canvas_bb.Min = center - ImVec2(maxRadius, maxRadius);
		canvas_bb.Max = center + ImVec2(maxRadius, maxRadius);
		data_bb = ImRect(ImVec2(-ylim.y, -ylim.y), ImVec2(ylim.y, ylim.y));
		break;
	}
	default:
		canvas_bb = ImRect(axes_bb.Min + ImVec2(yval_tsize.x + ylabel_tsize.x, 0.5f * yval_tsize.y), axes_bb.Max - ImVec2(0.5f * xval_tsize.x, xval_tsize.y + xlabel_tsize.y));
		data_bb = ImRect(ImVec2(xlim.x, ylim.x), ImVec2(xlim.y, ylim.y));
		break;
	}

	for (std::shared_ptr<ImPlot<T>> imPlot : vPlots)
	{
		imPlot->axesType = axesCoordType;
		imPlot->data_bb = data_bb;
		imPlot->canvas_bb = canvas_bb;
		imPlot->Render();
	}

	switch (axesCoordType)
	{
	case ImPlotCoordType_Polar:
	{
		ImVec2 center = canvas_bb.GetCenter();
		float maxRadius = 0.5f * canvas_bb.GetWidth();

		// Theta-Axis
		for (int i = 0; i < xtickNum; i++) {
			float rad = radians(360.f * i / xtickNum);
			ImVec2 norm = ImVec2(cos(rad), -sin(rad));
			ImVec2 offset = norm * maxRadius;
			if (yGridOn) {
				window->DrawList->AddLine(
					center,
					center + offset,
					gridColor,
					gridWidth);
			}
			std::string temp = ntos(degrees(rad), xPrecision);
			ImVec2 tSize = CalcTextSize(temp.c_str());
			window->DrawList->AddText(
				center + offset - tSize * 0.5f + ImVec2(0.5f * tSize.x * cos(rad), -0.5f * tSize.y * sin(rad))
				, axesColor
				, temp.c_str());
		}

		// R-Axis
		for (int i = 0; i < ytickNum; i++) {
			if (i == ytickNum - 1) {
				window->DrawList->AddCircle(center, maxRadius, axesColor, 36, axesWidth);
			}
			else if (xGridOn && i > 0) {
				window->DrawList->AddCircle(center, maxRadius * i / (ytickNum - 1), gridColor, 36, gridWidth);
			}
			std::string temp = ntos((float)i * ylim.y / (ytickNum - 1), yPrecision);
			ImVec2 tSize = CalcTextSize(temp.c_str());
			window->DrawList->AddText(
				center + ImVec2(0.f, -maxRadius * i / (ytickNum - 1))
				, axesColor
				, temp.c_str());
		}
		break;
	}
	default:
	{
		if (ylabel != L"") {
			// Calculate Center Position
			ImVec2 pos = ImVec2(axes_bb.Min.x, 0.5f * (canvas_bb.Max.y + canvas_bb.Min.y + ylabel_tsize.y));
			AddRotateText(pos, -90.f, axesColor, ylchar);
		}

		if (xlabel != L"") {
			ImVec2 pos = ImVec2(0.5 * (canvas_bb.Min.x + canvas_bb.Max.x - xlabel_tsize.x), axes_bb.Max.y - xlabel_tsize.y);
			window->DrawList->AddText(pos, axesColor, xlchar, NULL);
		}

		// Y-Axis
		for (int i = 0; i < ytickNum; i++) {
			if (i == 0 | i == ytickNum - 1) {
				window->DrawList->AddLine(
					ImVec2(canvas_bb.Min.x, canvas_bb.Max.y - (float)i * canvas_bb.GetSize().y / (ytickNum - 1)),
					canvas_bb.Max - ImVec2(0, (float)i * canvas_bb.GetSize().y / (ytickNum - 1)),
					axesColor,
					axesWidth);
			}
			else if (yGridOn) {
				window->DrawList->AddLine(
					ImVec2(canvas_bb.Min.x, canvas_bb.Max.y - (float)i * canvas_bb.GetSize().y / (ytickNum - 1)),
					canvas_bb.Max - ImVec2(0, (float)i * canvas_bb.GetSize().y / (ytickNum - 1)),
					gridColor,
					gridWidth);
			}
			std::string temp = ntos((float)i * (ylim.y - ylim.x) / (ytickNum - 1) + ylim.x, yPrecision);
			ImVec2 tSize = CalcTextSize(temp.c_str());
			window->DrawList->AddText(ImVec2(canvas_bb.Min.x - tSize.x, canvas_bb.Max.y - (float)i * canvas_bb.GetSize().y / (ytickNum - 1) - 0.5f * tSize.y), axesColor, temp.c_str());
		}

		// X-Axis
		for (int i = 0; i < xtickNum; i++) {
			if (i == 0 | i == xtickNum - 1) {
				window->DrawList->AddLine(
					ImVec2(canvas_bb.Min.x + (float)i * canvas_bb.GetSize().x / (xtickNum - 1), canvas_bb.Max.y),
					canvas_bb.Min + ImVec2((float)i * canvas_bb.GetSize().x / (xtickNum - 1), 0),
					axesColor,
					axesWidth);
			}
			else if (xGridOn) {
				window->DrawList->AddLine(
					ImVec2(canvas_bb.Min.x + (float)i * canvas_bb.GetSize().x / (xtickNum - 1), canvas_bb.Max.y),
					canvas_bb.Min + ImVec2((float)i * canvas_bb.GetSize().x / (xtickNum - 1), 0),
					gridColor,
					gridWidth);
			}
			std::string temp = ntos((float)i * (xlim.y - xlim.x) / (xtickNum - 1) + xlim.x, xPrecision);
			ImVec2 tSize = CalcTextSize(temp.c_str());
			window->DrawList->AddText(ImVec2(canvas_bb.Min.x + (float)i * canvas_bb.GetSize().x / (xtickNum - 1) - 0.5f * tSize.x, canvas_bb.Max.y + 3.f), axesColor, temp.c_str());
		}
		break;
	}
	}
}