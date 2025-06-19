import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    configuration_directory = LaunchConfiguration('configuration_directory')
    configuration_basename = LaunchConfiguration('configuration_basename')
    publish_period_sec = LaunchConfiguration('publish_period_sec', default='1.0')

    share_dir = get_package_share_directory('sllidar_ros2')
    rviz_config_file = os.path.join(share_dir, 'rviz', 'sllidar_cartographer.rviz')

    return LaunchDescription([
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use simulation time if true'),

        DeclareLaunchArgument(
            'configuration_directory',
            default_value=os.path.join(share_dir, 'lua'),
            description='Directory containing Cartographer configuration files'),

        DeclareLaunchArgument(
            'configuration_basename',
            default_value='sllidar_cartographer.lua',
            description='Cartographer configuration file'),

        DeclareLaunchArgument(
            'publish_period_sec',
            default_value='1.0',
            description='OccupancyGrid publishing period'),

        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_file],
            output='screen'
        ),

        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            output='screen',
            arguments=['0.0', '0.0', '0.0', '0.0', '0.0', '0.0', 'map', 'odom_rf2o']
        ),

        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            output='screen',
            arguments=['0.0', '0.0', '0.0', '0.0', '0.0', '0.0', 'odom_rf2o', 'base_footprint']
        ),

        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            output='screen',
            arguments=['0.0', '0.0', '0.0', '0.0', '0.0', '0.0', 'base_link', 'laser']
        ),

        Node(
            package='cartographer_ros',
            executable='cartographer_node',
            name='cartographer_node',
            output='screen',
            arguments=[
                '-configuration_directory', '/home/ryoma/RP_ws/src/sllidar_ros2/lua',
                '-configuration_basename', 'sllidar_cartographer.lua'
            ],
            remappings=[('odom', 'odom_rf2o')]
        ),

        Node(
            package='cartographer_ros',
            #executable='cartographer_node',
            #name='cartographer_node',
            #output='log',
            executable='cartographer_occupancy_grid_node',
            name='occupancy_grid_node',
            output='screen',
            parameters=[{'use_sim_time': use_sim_time}],
            arguments=[
                '-configuration_directory', '/home/ryoma/RP_ws/src/sllidar_ros2/lua',
                '-configuration_basename', 'sllidar_cartographer.lua'
            ],
            remappings=[('odom', 'odom_rf2o')]
        ),
    ])
