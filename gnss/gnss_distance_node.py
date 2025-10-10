import rclpy
from rclpy.node import Node
from sensor_msgs.msg import NavSatFix
#from geopy.distance import geodesic

class GPSDistanceCalculator(Node):
    def __init__(self):
        super().__init__('gps_distance_calculator')
        self.subscription = self.create_subscription(
            NavSatFix,
            '/fix',
            self.listener_callback,
            10)
        self.prev_coord = None

    def listener_callback(self, msg):
        #current_coord = (msg.latitude, msg.longitude)
        #if self.prev_coord:
        #    distance = geodesic(self.prev_coord, current_coord).meters
        #    self.get_logger().info(f"移動距離: {distance:.2f} m")
        #self.prev_coord = current_coord
        
        latitude = msg.latitude
        longitude = msg.longitude
        self.get_logger().info(f"緯度: {latitude:.6f}, 経度: {longitude:.6f}")


def main(args=None):
    rclpy.init(args=args)
    node = GPSDistanceCalculator()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
