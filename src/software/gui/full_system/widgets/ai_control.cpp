#include "software/gui/full_system/widgets/ai_control.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>

#include "shared/parameter/cpp_dynamic_parameters.h"
#include "software/ai/hl/stp/play/play.h"
#include "software/gui/shared/parameters_spinbox.h"
#include "software/logger/logger.h"
#include "software/util/design_patterns/generic_factory.h"

void setupAIControls(Ui::AutogeneratedFullSystemMainWidget *widget,
                     std::shared_ptr<ThunderbotsConfig> config)
{
    setupAIStartAndStopButtons(widget,
                               config->getMutableAiControlConfig()->getMutableRunAi());
    setupTeamColourComboBox(
        widget->team_colour_combo_box,
        config->getMutableSensorFusionConfig()->getMutableFriendlyColorYellow());
    setupDefendingSideComboBox(
        widget->defending_side_combo_box,
        config->getMutableSensorFusionConfig()
            ->getMutableOverrideGameControllerDefendingSide(),
        config->getMutableSensorFusionConfig()->getMutableDefendingPositiveSide());
    setupGameStateOverrideComboBox(
        widget->game_state_override_combo_box,
        config->getMutableSensorFusionConfig()->getMutableOverrideRefereeCommand(),
        config->getMutableSensorFusionConfig()->getMutablePreviousRefereeCommand(),
        config->getMutableSensorFusionConfig()->getMutableCurrentRefereeCommand());
    setupPlayOverrideComboBox(
        widget->play_override_combo_box,
        config->getMutableAiControlConfig()->getMutableOverrideAiPlay(),
        config->getMutableAiControlConfig()->getMutableCurrentAiPlay());
    setupChannelSpinBox(widget->channel_spin_box,
                        config->getMutableNetworkConfig()->getMutableChannel());
    setupGoalieIdComboBox(
        widget->friendly_goalie_id_combo_box,
        config->getMutableSensorFusionConfig()
            ->getMutableOverrideGameControllerFriendlyGoalieId(),
        config->getMutableSensorFusionConfig()->getMutableFriendlyGoalieId());
    setupGoalieIdComboBox(
        widget->enemy_goalie_id_combo_box,
        config->getMutableSensorFusionConfig()
            ->getMutableOverrideGameControllerEnemyGoalieId(),
        config->getMutableSensorFusionConfig()->getMutableEnemyGoalieId());
}

void setupAIStartAndStopButtons(Ui::AutogeneratedFullSystemMainWidget *widget,
                                std::shared_ptr<Parameter<bool>> run_ai_parameter)
{
    auto start_ai_func = [run_ai_parameter]() { run_ai_parameter->setValue(true); };
    QWidget::connect(widget->start_ai_button, &QPushButton::clicked, start_ai_func);
    auto stop_ai_func = [run_ai_parameter]() { run_ai_parameter->setValue(false); };
    QWidget::connect(widget->stop_ai_button, &QPushButton::clicked, stop_ai_func);
}

void setupTeamColourComboBox(
    QComboBox *team_colour_combo_box,
    std::shared_ptr<Parameter<bool>> friendly_colour_yellow_parameter)
{
    team_colour_combo_box->insertItem(0, "Yellow");
    team_colour_combo_box->insertItem(1, "Blue");
    auto on_team_colour_changed =
        [friendly_colour_yellow_parameter](const QString &text) {
            if (text == "Yellow")
            {
                friendly_colour_yellow_parameter->setValue(true);
            }
            else if (text == "Blue")
            {
                friendly_colour_yellow_parameter->setValue(false);
            }
            else
            {
                LOG(FATAL) << "Tried to set the team colour with an invalid value: '"
                           << text.toStdString() << "'" << std::endl;
            }
        };
    QWidget::connect(team_colour_combo_box, &QComboBox::currentTextChanged,
                     on_team_colour_changed);

    auto on_parameter_value_changed = [team_colour_combo_box](bool new_value) {
        // We block signals while setting the state of the checkbox so that we don't
        // trigger the `on_checkbox_value_changed` function, which would set the
        // parameter value again and deadlock on the parameter's internal mutex
        team_colour_combo_box->blockSignals(true);
        team_colour_combo_box->setCurrentIndex(new_value ? 0 : 1);
        team_colour_combo_box->blockSignals(false);
    };
    friendly_colour_yellow_parameter->registerCallbackFunction(on_parameter_value_changed);

    on_parameter_value_changed(friendly_colour_yellow_parameter->value());
}

void setupDefendingSideComboBox(
    QComboBox *defending_side_combo_box,
    std::shared_ptr<Parameter<bool>> defending_side_override_parameter,
    std::shared_ptr<Parameter<bool>> defending_positive_side_parameter)
{
    // See issue #811 for getting these value from an enum / factory
    defending_side_combo_box->insertItem(0, "Use GameController");
    defending_side_combo_box->insertItem(1, "Positive");
    defending_side_combo_box->insertItem(2, "Negative");

    auto on_defending_side_changed =
        [defending_side_override_parameter,
         defending_positive_side_parameter](const QString &text) {
            if (text == "Use GameController")
            {
                defending_side_override_parameter->setValue(false);
            }
            else if (text == "Positive")
            {
                defending_side_override_parameter->setValue(true);
                defending_positive_side_parameter->setValue(true);
            }
            else if (text == "Negative")
            {
                defending_side_override_parameter->setValue(true);
                defending_positive_side_parameter->setValue(false);
            }
            else
            {
                LOG(FATAL) << "Tried to set the defending side with an invalid value: '"
                           << text.toStdString() << "'" << std::endl;
            }
        };
    QWidget::connect(defending_side_combo_box, &QComboBox::currentTextChanged,
                     on_defending_side_changed);

    auto on_parameter_value_changed = [=]() {
        // We block signals while setting the state of the checkbox so that we don't
        // trigger the `on_checkbox_value_changed` function, which would set the
        // parameter value again and deadlock on the parameter's internal mutex
        defending_side_combo_box->blockSignals(true);
        if(defending_side_override_parameter->value()) {
            defending_side_combo_box->setCurrentIndex(defending_positive_side_parameter->value() ? 1 : 2);
        }else{
            defending_side_combo_box->setCurrentIndex(0);
        }
        defending_side_combo_box->blockSignals(false);
    };
    defending_positive_side_parameter->registerCallbackFunction([=](bool unused){on_parameter_value_changed();});
    defending_side_override_parameter->registerCallbackFunction([=](bool unused){on_parameter_value_changed();});

    on_parameter_value_changed();
}

void setupGameStateOverrideComboBox(
    QComboBox *game_state_override_combo_box,
    std::shared_ptr<Parameter<bool>> game_state_override_parameter,
    std::shared_ptr<Parameter<std::string>> previous_game_state_parameter,
    std::shared_ptr<Parameter<std::string>> current_game_state_parameter)
{
    const QString gamecontroller_text = "Use GameController";

    game_state_override_combo_box->addItem(gamecontroller_text);
    for (const auto &state : allValuesRefereeCommand())
    {
        game_state_override_combo_box->addItem(QString::fromStdString(toString(state)));
    }

    auto on_game_state_changed = [=](const QString &text) {
        if (text == gamecontroller_text)
        {
            game_state_override_parameter->setValue(false);
        }
        else
        {
            game_state_override_parameter->setValue(true);
            previous_game_state_parameter->setValue(
                current_game_state_parameter->value());
            current_game_state_parameter->setValue(text.toStdString());
        }
    };
    QWidget::connect(game_state_override_combo_box, &QComboBox::currentTextChanged,
                     on_game_state_changed);

    auto on_parameter_value_changed = [=]() {
        // We block signals while setting the state of the checkbox so that we don't
        // trigger the `on_checkbox_value_changed` function, which would set the
        // parameter value again and deadlock on the parameter's internal mutex
        game_state_override_combo_box->blockSignals(true);
        if(game_state_override_parameter->value()) {
            game_state_override_combo_box->setCurrentText(QString::fromStdString(current_game_state_parameter->value()));
        }else {
            game_state_override_combo_box->setCurrentText(gamecontroller_text);
        }
        game_state_override_combo_box->blockSignals(false);
    };
    game_state_override_parameter->registerCallbackFunction([=](bool unused){on_parameter_value_changed();});
    current_game_state_parameter->registerCallbackFunction([=](std::string unused){on_parameter_value_changed();}) ;

    on_parameter_value_changed();
}

void setupPlayOverrideComboBox(
    QComboBox *play_override_combo_box,
    std::shared_ptr<Parameter<bool>> play_override_parameter,
    std::shared_ptr<Parameter<std::string>> current_play_parameter)
{
    const QString ai_selection_text = "Use AI Selection";

    auto play_names = GenericFactory<std::string, Play, PlayConfig>::getRegisteredNames();

    // Sort the entries in alphabetical order from a-z
    std::sort(play_names.begin(), play_names.end());

    // Create a new list with all the play names converted to QStrings
    QList<QString> qt_play_names;
    std::transform(play_names.begin(), play_names.end(),
                   std::back_inserter(qt_play_names),
                   [](std::string name) { return QString::fromStdString(name); });

    play_override_combo_box->insertItem(0, ai_selection_text);
    play_override_combo_box->insertItems(1, qt_play_names);

    auto on_play_changed = [=](const QString &text) {
        if (text == ai_selection_text)
        {
            play_override_parameter->setValue(false);
        }
        else
        {
            play_override_parameter->setValue(true);
            current_play_parameter->setValue(text.toStdString());
        }
    };
    QWidget::connect(play_override_combo_box, &QComboBox::currentTextChanged,
                     on_play_changed);

    auto on_parameter_value_changed = [=]() {
        // We block signals while setting the state of the checkbox so that we don't
        // trigger the `on_checkbox_value_changed` function, which would set the
        // parameter value again and deadlock on the parameter's internal mutex
        play_override_combo_box->blockSignals(true);
        if(play_override_parameter->value()) {
            play_override_combo_box->setCurrentText(QString::fromStdString(current_play_parameter->value()));
        }else {
            play_override_combo_box->setCurrentText(ai_selection_text);
        }
        play_override_combo_box->blockSignals(false);
    };
    play_override_parameter->registerCallbackFunction([=](bool unused){on_parameter_value_changed();});
    current_play_parameter->registerCallbackFunction([=](std::string unused){on_parameter_value_changed();}) ;

    on_parameter_value_changed();
}

void setupChannelSpinBox(QSpinBox *channel_spin_box,
                         std::shared_ptr<NumericParameter<int>> channel_parameter)
{
    channel_spin_box->setValue(channel_parameter->value());
    channel_spin_box->setMinimum(0);
    channel_spin_box->setMaximum(MAX_MULTICAST_CHANNELS);

    auto on_channel_changed = [channel_parameter](int new_value) {
        channel_parameter->setValue(new_value);
    };

    // QSpinBox has 2 "valueChanged" signals that each provide different info (string vs
    // int), so we need to static_cast to specify the integer version
    QWidget::connect(channel_spin_box,
                     static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                     on_channel_changed);

    setupSpinBox(channel_spin_box, channel_parameter);

    auto on_parameter_value_changed = [channel_spin_box](int new_value) {
        // We block signals while setting the state of the checkbox so that we don't
        // trigger the `on_checkbox_value_changed` function, which would set the
        // parameter value again and deadlock on the parameter's internal mutex
        channel_spin_box->blockSignals(true);
        channel_spin_box->setValue(new_value);
        channel_spin_box->blockSignals(false);

    };

    on_parameter_value_changed(channel_parameter->value());
}

void setupGoalieIdComboBox(QComboBox *id_comboBox,
                           std::shared_ptr<Parameter<bool>> override_goalie_id_parameter,
                           std::shared_ptr<NumericParameter<int>> goalie_id_parameter)
{
    QList<QString> robot_ids;
    for (unsigned int id = 0; id < MAX_ROBOT_IDS; id++)
    {
        QString str = QString::number(id);
        robot_ids.insert(id, str);
    }
    id_comboBox->insertItem(0, "Use GameController");
    id_comboBox->insertItems(1, (robot_ids));
    // TODO: this value is hardcoded and it should take the default value from dynamic
    // parameters once https://github.com/UBC-Thunderbots/Software/issues/1299 is done and
    // integrated
    id_comboBox->setCurrentText(QString::fromStdString("0"));

    auto on_goalie_id_changed = [override_goalie_id_parameter, goalie_id_parameter,
                                 robot_ids](const QString &text) {
        if (text == "Use GameController")
        {
            override_goalie_id_parameter->setValue(false);
        }
        else if (robot_ids.contains(text))
        {
            override_goalie_id_parameter->setValue(true);
            goalie_id_parameter->setValue(text.toInt());
        }
        else
        {
            LOG(FATAL) << "Tried to set the goalie ID with an invalid value: '"
                       << text.toStdString() << "'" << std::endl;
        }
    };

    QWidget::connect(id_comboBox, &QComboBox::currentTextChanged, on_goalie_id_changed);
}
