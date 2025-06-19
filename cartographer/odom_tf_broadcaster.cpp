#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

using namespace std::chrono_literals;

class OdomTfBroadcaster : public rclcpp::Node
{
public:
  OdomTfBroadcaster()
  : Node("odom_tf_broadcaster")
  {
    // TF broadcaster 初期化
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

    // odom トピックサブスクライブ
    odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "odom", 10,
      std::bind(&OdomTfBroadcaster::odomCallback, this, std::placeholders::_1));
  }

private:
  void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    geometry_msgs::msg::TransformStamped t;

    t.header.stamp = this->now();
    t.header.frame_id = "odom";
    t.child_frame_id = "base_link";

    t.transform.translation.x = msg->pose.pose.position.x;
    t.transform.translation.y = msg->pose.pose.position.y;
    t.transform.translation.z = msg->pose.pose.position.z;

    t.transform.rotation = msg->pose.pose.orientation;

    // tfを配信
    tf_broadcaster_->sendTransform(t);
  }

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<OdomTfBroadcaster>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
