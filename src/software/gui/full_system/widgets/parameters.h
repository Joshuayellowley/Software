#pragma once

#include <QtWidgets/QWidget>

#include "software/parameter/config.h"
#include "software/parameter/dynamic_parameters.h"
#include "software/parameter/parameter.h"
// This include is autogenerated by the .ui file in the same folder
// The generated version will be names 'ui_<filename>.h'
#include "software/gui/full_system/ui/ui_main_widget.h"

/**
 * Sets up the Parameters tab that contains all the tuneable parameters for the AI
 */
void setupParametersTab(Ui::AutoGeneratedMainWidget* widget);

/**
 * Creates a widget that contains the components necessary to display and control a
 * boolean Parameter for the AI
 *
 * @param parameter The boolean parameter to create a widget for
 * @return A pointer to the QWidget that will be used to control the given parameter
 */
QWidget* createBooleanParameter(std::shared_ptr<Parameter<bool>> parameter);

/**
 * Creates a widget that contains the components necessary to display and control an
 * integer Parameter for the AI
 *
 * @param parameter The integer parameter to create a widget for
 * @return A pointer to the QWidget that will be used to control the given parameter
 */
QWidget* createIntegerParameter(std::shared_ptr<Parameter<int>> parameter);

/**
 * Creates a widget that contains the components necessary to display and control a
 * double Parameter for the AI
 *
 * @param parameter The double parameter to create a widget for
 * @return A pointer to the QWidget that will be used to control the given parameter
 */
QWidget* createDoubleParameter(std::shared_ptr<Parameter<double>> parameter);

/**
 * Creates a widget that contains the components necessary to display and control a
 * string Parameter for the AI
 *
 * @param parameter The string parameter to create a widget for
 * @return A pointer to the QWidget that will be used to control the given parameter
 */
QWidget* createStringParameter(std::shared_ptr<Parameter<std::string>> parameter);