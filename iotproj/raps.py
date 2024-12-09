import board
import busio
import time
import threading
import json
import sys
from http.server import BaseHTTPRequestHandler, HTTPServer

import neopixel

import adafruit_veml7700
import adafruit_tcs34725

# device setup
i2c = busio.I2C(board.SCL, board.SDA)
veml7700 = adafruit_veml7700.VEML7700(i2c)
tcs = adafruit_tcs34725.TCS34725(i2c)
tcs.integration_time = 100
tcs.gain = 1
pixel_pin = board.D18  
num_pixels = 7
pixels = neopixel.NeoPixel(pixel_pin, num_pixels, brightness=1.0, auto_write=False)

#value template
target_values = {
    'lux': None,
    'r': None,
    'g': None,
    'b': None,
}


target_lock = threading.Lock()

# auto adjust
def adjust_neopixels():

    current_pixel_values = [0, 0, 0]
    while True:
        with target_lock:
            if not all(value is not None for value in target_values.values()):
                time.sleep(0.1)
                continue

            
            current_lux = veml7700.lux
            current_r, current_g, current_b = tcs.color_rgb_bytes

            print('Current Sensor Readings:')
            print(f'Lux: {current_lux}')
            print(f'RGB: {current_r}, {current_g}, {current_b}')

      
            lux_error = target_values['lux'] - current_lux
            r_error = target_values['r'] - current_r
            g_error = target_values['g'] - current_g
            b_error = target_values['b'] - current_b

            
            k_lux = 0.01  
            k_rgb = 0.1  

            brightness_adjustment = k_lux * lux_error
            new_brightness = pixels.brightness + brightness_adjustment
            pixels.brightness = min(max(new_brightness, 0.0), 1.0)

           
            adjusted_r = current_pixel_values[0] + k_rgb * r_error
            adjusted_g = current_pixel_values[1] + k_rgb * g_error
            adjusted_b = current_pixel_values[2] + k_rgb * b_error

            
            adjusted_r = int(min(max(adjusted_r, 0), 255))
            adjusted_g = int(min(max(adjusted_g, 0), 255))
            adjusted_b = int(min(max(adjusted_b, 0), 255))

            
            current_pixel_values = [adjusted_r, adjusted_g, adjusted_b]

            for i in range(num_pixels):
                pixels[i] = (adjusted_r, adjusted_g, adjusted_b)
            pixels.show()

        time.sleep(0.1)

# takt the value from the devices
class RequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length)
        try:
           
            json_data = json.loads(post_data)
            
            print("JSON data:")
            print(json.dumps(json_data, indent=4))

            with target_lock:
               
                if 'lux' in json_data:
                    target_values['lux'] = json_data['lux']
                if 'r' in json_data:
                    target_values['r'] = json_data['r']
                if 'g' in json_data:
                    target_values['g'] = json_data['g']
                if 'b' in json_data:
                    target_values['b'] = json_data['b']

           
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b'JSON data received successfully.')
        except json.JSONDecodeError:
            
            self.send_response(400)
            self.end_headers()
            self.wfile.write(b'Invalid JSON data received.')
           

    def log_message(self, format, *args):
       
        return

def run(server_class=HTTPServer, handler_class=RequestHandler, port=8000):
    
    server_address = ('', port)  
    httpd = server_class(server_address, handler_class)
    print(f'Starting HTTP server on port {port}...')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print('Shutting down server.')
        httpd.server_close()
        
        pixels.fill((0, 0, 0))
        pixels.show()
        sys.exit(0)

adjustment_thread = threading.Thread(target=adjust_neopixels)
adjustment_thread.daemon = True
adjustment_thread.start()

if __name__ == '__main__':
    run()
