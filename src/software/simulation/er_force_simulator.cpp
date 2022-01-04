#include "software/simulation/er_force_simulator.h"

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>

#include <QtCore/QFile>
#include <QtCore/QString>
#include <iostream>

#include "extlibs/er_force_sim/src/protobuf/robot.h"
#include "proto/message_translation/primitive_google_to_nanopb_converter.h"
#include "proto/message_translation/ssl_detection.h"
#include "proto/message_translation/ssl_geometry.h"
#include "proto/message_translation/ssl_simulation_robot_control.h"
#include "proto/message_translation/ssl_wrapper.h"
#include "proto/message_translation/tbots_protobuf.h"
#include "software/world/robot_state.h"

ErForceSimulator::ErForceSimulator(
    const Field& field, const RobotConstants_t& robot_constants,
    const WheelConstants& wheel_constants,
    std::shared_ptr<const SimulatorConfig> simulator_config)
    : yellow_team_vision_msg(std::make_unique<TbotsProto::Vision>()),
      blue_team_vision_msg(std::make_unique<TbotsProto::Vision>()),
      frame_number(0),
      robot_constants(robot_constants),
      wheel_constants(wheel_constants)
{
    QString full_filename = CONFIG_DIRECTORY + CONFIG_FILE + ".txt";
    QFile file(full_filename);
    if (!file.open(QFile::ReadOnly))
    {
        LOG(FATAL) << "Could not open configuration file " << full_filename.toStdString()
                   << std::endl;
    }
    QString str = file.readAll();
    file.close();
    std::string s = qPrintable(str);

    google::protobuf::TextFormat::Parser parser;
    parser.ParseFromString(s, &er_force_sim_setup);

    er_force_sim = std::make_unique<camun::simulator::Simulator>(er_force_sim_setup);

    auto simulator_setup_command = std::make_unique<amun::Command>();
    simulator_setup_command->mutable_simulator()->set_enable(true);
    // start with default robots, take ER-Force specs.
    robot::Specs ERForce;
    robotSetDefault(&ERForce);

    Team friendly_team = Team();
    Team enemy_team    = Team();
    Ball ball          = Ball(Point(), Vector(), Timestamp::fromSeconds(0));

    World world = World(field, ball, friendly_team, enemy_team);

    // TODO (#2283): remove this initialization when addYellowRobots and addBlueRobots are
    // implemented

    // TODO (#2283): remove this initialization when addYellowRobots and addBlueRobots are
    // implemented

    this->resetCurrentFirmwareTime();
}

void ErForceSimulator::setBallState(const BallState& ball_state)
{
    er_force_sim->safelyTeleportBall(static_cast<float>(ball_state.position().x()),
                                     static_cast<float>(ball_state.position().y()));
}

void ErForceSimulator::addYellowRobots(const std::vector<RobotStateWithId>& robots)
{
    // TODO (#2283): add robots

    auto simulator_setup_command = std::make_shared<amun::Command>();
    simulator_setup_command->mutable_simulator()->set_enable(true);

    robot::Specs ERForce;
    robotSetDefault(&ERForce);

    auto* team_yellow = simulator_setup_command->mutable_set_team_yellow();
    std::vector<std::tuple<int, int>>
        yellow_robot_pos;  //=new std::vector<std::tuple<int, int>>;


    for (unsigned int i = 0; i < robots.size(); i++)
    {
        auto* robot = team_yellow->add_robot();
        robot->CopyFrom(ERForce);
        robot->set_id(i);
        yellow_robot_pos.push_back(std::make_tuple(robots[i].robot_state.position().y(),
                                                   robots[i].robot_state.position().x()));
    }

    er_force_sim->handleSimulatorSetupCommand(simulator_setup_command, yellow_robot_pos);
    er_force_sim->seedPRGN(17);

    for (unsigned int i = 0; i < robots.size(); i++)
    {
        auto yellow_simulator_robot = std::make_shared<ErForceSimulatorRobot>(
            RobotStateWithId{.id = i, .robot_state = robots[i].robot_state},
            robot_constants, wheel_constants);

        blue_simulator_robots.emplace_back(blue_simulator_robot);
        yellow_simulator_robots.emplace_back(yellow_simulator_robot);
    }

    this->resetCurrentTime();
}

void ErForceSimulator::setBallState(const BallState& ball_state)
{
    auto simulator_setup_command = std::make_unique<amun::Command>();
    auto teleport_ball           = std::make_unique<sslsim::TeleportBall>();
    auto simulator_control       = std::make_unique<sslsim::SimulatorControl>();
    auto command_simulator       = std::make_unique<amun::CommandSimulator>();

    teleport_ball->set_x(
        static_cast<float>(ball_state.position().x() * MILLIMETERS_PER_METER));
    teleport_ball->set_y(
        static_cast<float>(ball_state.position().y() * MILLIMETERS_PER_METER));
    teleport_ball->set_vx(
        static_cast<float>(ball_state.velocity().x() * MILLIMETERS_PER_METER));
    teleport_ball->set_vy(
        static_cast<float>(ball_state.velocity().y() * MILLIMETERS_PER_METER));
    *(simulator_control->mutable_teleport_ball())   = *teleport_ball;
    *(command_simulator->mutable_ssl_control())     = *simulator_control;
    *(simulator_setup_command->mutable_simulator()) = *command_simulator;


    er_force_sim->handleSimulatorSetupCommand(simulator_setup_command);
}

void ErForceSimulator::addYellowRobots(const std::vector<RobotStateWithId>& robots)
{
    // TODO (#2283): add robots
}

void ErForceSimulator::addBlueRobots(const std::vector<RobotStateWithId>& robots)
{
    // TODO (#2283): add robots
    auto simulator_setup_command = std::make_shared<amun::Command>();
    simulator_setup_command->mutable_simulator()->set_enable(true);

    robot::Specs ERForce;
    robotSetDefault(&ERForce);

    auto team_blue = simulator_setup_command->mutable_set_team_blue();
    std::vector<std::tuple<int, int>> blue_robot_pos;
    // = new std::vector<std::tuple<int, int>>;

    for (unsigned int i = 0; i < robots.size(); i++)
    {
        auto* robot = team_blue->add_robot();
        robot->CopyFrom(ERForce);
        robot->set_id(i);
        blue_robot_pos.push_back(std::make_tuple(robots[i].robot_state.position().y(),
                                                 robots[i].robot_state.position().x()));
    }

    er_force_sim->handleSimulatorSetupCommand(simulator_setup_command, blue_robot_pos);
    er_force_sim->seedPRGN(17);

    for (unsigned int i = 0; i < robots.size(); i++)
    {
        auto blue_simulator_robot = std::make_shared<ErForceSimulatorRobot>(
            RobotStateWithId{.id = i, .robot_state = robots[i].robot_state},
            robot_constants, wheel_constants);

        auto blue_firmware_robot = ErForceSimulatorRobotSingleton::createFirmwareRobot();
        auto blue_firmware_ball  = SimulatorBallSingleton::createFirmwareBall();

        FirmwareWorld_t* blue_firmware_world_raw = app_firmware_world_create(
            blue_firmware_robot.release(), blue_firmware_ball.release(),
            &(ErForceSimulator::getCurrentFirmwareTimeSeconds));
        auto blue_firmware_world = std::shared_ptr<FirmwareWorld_t>(
            blue_firmware_world_raw, FirmwareWorldDeleter());

        blue_simulator_robots.insert(
            std::make_pair(blue_simulator_robot, blue_firmware_world));
    }
}

void ErForceSimulator::setYellowRobotPrimitiveSet(
    const TbotsProto::PrimitiveSet& primitive_set_msg,
    std::unique_ptr<TbotsProto::Vision> vision_msg)
{
    for (auto& [robot_id, primitive] : primitive_set_msg.robot_primitives())
    {
        setRobotPrimitive(robot_id, primitive, yellow_simulator_robots,
                          *yellow_team_vision_msg);
    }
    yellow_team_vision_msg = std::move(vision_msg);
}

void ErForceSimulator::setBlueRobotPrimitiveSet(
    const TbotsProto::PrimitiveSet& primitive_set_msg,
    std::unique_ptr<TbotsProto::Vision> vision_msg)
{
    for (auto& [robot_id, primitive] : primitive_set_msg.robot_primitives())
    {
        setRobotPrimitive(robot_id, primitive, blue_simulator_robots,
                          *blue_team_vision_msg);
    }
    blue_team_vision_msg = std::move(vision_msg);
}

void ErForceSimulator::setRobotPrimitive(
    RobotId id, const TbotsProto::Primitive& primitive_msg,
    std::vector<std::shared_ptr<ErForceSimulatorRobot>>& simulator_robots,
    const TbotsProto::Vision& vision_msg)
{
    // Set to NEG_X because the vision msg in this simulator is normalized
    // correctly
    auto simulator_robots_iter =
        std::find_if(simulator_robots.begin(), simulator_robots.end(),
                     [id](const auto& robot) { return robot->getRobotId() == id; });

    if (simulator_robots_iter != simulator_robots.end())
    {
        auto simulator_robot = *simulator_robots_iter;

        auto robot_state_it = vision_msg.robot_states().find(id);
        if (robot_state_it != vision_msg.robot_states().end())
        {
            simulator_robot->setRobotState(
                createRobotState(vision_msg.robot_states().at(id)));
            simulator_robot->startNewPrimitive(primitive_msg);
        }
    }
    else
    {
        LOG(WARNING) << "Simulator robot with ID " << id << " not found" << std::endl;
    }
}

SSLSimulationProto::RobotControl ErForceSimulator::updateSimulatorRobots(
    std::vector<std::shared_ptr<ErForceSimulatorRobot>> simulator_robots,
    TbotsProto::Vision vision_msg)
{
    SSLSimulationProto::RobotControl robot_control;

    for (auto& simulator_robot : simulator_robots)
    {
        auto robot_state_it =
            vision_msg.robot_states().find(simulator_robot->getRobotId());
        if (robot_state_it != vision_msg.robot_states().end())
        {
            simulator_robot->setRobotState(createRobotState(
                vision_msg.robot_states().at(simulator_robot->getRobotId())));
            // Set to NEG_X because the vision msg in this simulator is
            // normalized correctly
            simulator_robot->runCurrentPrimitive();
            auto command = *simulator_robot->getRobotCommand();
            *(robot_control.mutable_robot_commands()->Add()) = command;
        }
    }
    return robot_control;
}

void ErForceSimulator::stepSimulation(const Duration& time_step)
{
    current_time = current_time + time_step;

    SSLSimulationProto::RobotControl yellow_robot_control =
        updateSimulatorRobots(yellow_simulator_robots, *yellow_team_vision_msg);

    SSLSimulationProto::RobotControl blue_robot_control =
        updateSimulatorRobots(blue_simulator_robots, *blue_team_vision_msg);

    er_force_sim->acceptYellowRobotControlCommand(yellow_robot_control);
    er_force_sim->acceptBlueRobotControlCommand(blue_robot_control);
    er_force_sim->stepSimulation(time_step.toSeconds());

    frame_number++;
}

std::vector<SSLProto::SSL_WrapperPacket> ErForceSimulator::getSSLWrapperPackets() const
{
    return er_force_sim->getWrapperPackets();
}

Field ErForceSimulator::getField() const
{
    return Field::createSSLDivisionAField();
}

Timestamp ErForceSimulator::getTimestamp() const
{
    return current_time;
}

void ErForceSimulator::resetCurrentTime()
{
    current_time = Timestamp::fromSeconds(0);
}
