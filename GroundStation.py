import pygame
import serial
import time

def float_to_byte(my_float):
    my_int = round(127.5*(my_float+1))
    return my_int.to_bytes(1, byteorder='big')

def wait_milliseconds(milliseconds):
    time.sleep(milliseconds/1000)

def main():
    pygame.init()
    pygame.joystick.init()

    if pygame.joystick.get_count() == 0:
        print("No game controller detected.")
        return

    joystick = pygame.joystick.Joystick(0)
    joystick.init()

    # print(joystick.get_numbuttons())

    my_serial = serial.Serial("COM5", baudrate=115200, timeout=1)

    try:
        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    return

            # Print stick and trigger positions
            # left_stick_x = joystick.get_axis(0)
            # left_stick_y = joystick.get_axis(1)
            # right_stick_x = joystick.get_axis(2)
            # right_stick_y = joystick.get_axis(3)

            # left_trigger = joystick.get_axis(4)
            # right_trigger = joystick.get_axis(5)

            # print(f"Left Stick: ({left_stick_x:.2f}, {left_stick_y:.2f})")

            # print(f"Right Stick: ({right_stick_x:.2f}, {right_stick_y:.2f})")
            # print(f"Left Trigger: {left_trigger:.2f}")
            # print(f"Right Trigger: {right_trigger:.2f}")
            # print("\n")

            # print(f"A: {joystick.get_button(0)}")
            # print(f"B: {joystick.get_button(1)}")
            # print(f"X: {joystick.get_button(2)}")
            # print(f"Y: {joystick.get_button(3)}")
            # print(f"L: {joystick.get_button(4)}")
            # print(f"R: {joystick.get_button(5)}")

            while my_serial.in_waiting > 0:
                print(my_serial.readline())
            
            # Send Sticks
            for i in range(6):
                my_serial.write((0).to_bytes(1, byteorder='big')) # zero
                my_serial.write((i+1).to_bytes(1, byteorder='big')) # field
                # print((i+1).to_bytes(1, byteorder='big'))

                payload = float_to_byte(joystick.get_axis(i))
                if (payload == b'\x00'):
                    payload = b'\x01'
                my_serial.write(payload)

            # Send Buttons
            for i in range(6):
                my_serial.write((0).to_bytes(1, byteorder='big')) # zero
                my_serial.write((i+7).to_bytes(1, byteorder='big')) # field
                # print((i+1).to_bytes(1, byteorder='big'))
                if (joystick.get_button(i) == 1):
                    my_serial.write(b'\xFF')
                else:
                    my_serial.write(b'\x01')

            pygame.time.delay(50)
            
            # received_data = my_serial.readline().decode('utf-8').strip()
            # print(f"Received data: {received_data}")

    except KeyboardInterrupt:
        pass
    finally:
        # Quit pygame
        pygame.quit()

if __name__ == "__main__":
    main()
