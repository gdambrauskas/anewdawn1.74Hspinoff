#include "CvGameCoreDLL.h"
#include "CvProperties.h"

//
// Python interface for CvProperties class
// simple enough to be exposed directly - no wrapper
//

void CyPropertiesPythonInterface()
{
	OutputDebugString("Python Extension Module - CyPropertiesPythonInterface\n");

	python::class_<CvProperties>("CvProperties")

		.def("getProperty", &CvProperties::getProperty, "int (int)")
		.def("getValue", &CvProperties::getValue, "int (int)")
		.def("getNumProperties", &CvProperties::getNumProperties, "int ()")
		.def("getValueByProperty", &CvProperties::getValueByProperty, "int (int)")
		.def("getPropertyDisplay", &CvProperties::getPropertyDisplay, "wstring (int)")
		;
}