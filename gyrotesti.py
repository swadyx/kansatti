import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
import numpy as np
import serial
import math

# Setup serial connection
ser = serial.Serial('/dev/ttyUSB0', 9600)

# Initialize PyGame
pygame.init()
display = (800, 600)
pygame.display.set_mode(display, DOUBLEBUF|OPENGL)
pygame.display.set_caption('CanSat Gyroscope Visualization')

# Set up the OpenGL environment
gluPerspective(45, (display[0]/display[1]), 0.1, 50.0)
glTranslatef(0.0, 0.0, -5)

# Function to draw coordinate axes
def draw_axes(size=2.0):
    glBegin(GL_LINES)
    # X axis (red)
    glColor3f(1.0, 0.0, 0.0)
    glVertex3f(0.0, 0.0, 0.0)
    glVertex3f(size, 0.0, 0.0)
    # Y axis (green)
    glColor3f(0.0, 1.0, 0.0)
    glVertex3f(0.0, 0.0, 0.0)
    glVertex3f(0.0, size, 0.0)
    # Z axis (blue)
    glColor3f(0.0, 0.0, 1.0)
    glVertex3f(0.0, 0.0, 0.0)
    glVertex3f(0.0, 0.0, size)
    glEnd()

# Main loop
running = True
roll, pitch, yaw = 0, 0, 0

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    
    # Read data from serial
    if ser.in_waiting:
        try:
            data = ser.readline().decode('utf-8').strip()
            roll, pitch, yaw = map(float, data.split(','))
        except:
            pass  # Skip malformed data
    
    # Clear the screen
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)
    
    # Save the current matrix
    glPushMatrix()
    
    # Apply rotations
    glRotatef(roll, 1, 0, 0)   # Roll around X axis
    glRotatef(pitch, 0, 1, 0)  # Pitch around Y axis
    glRotatef(yaw, 0, 0, 1)    # Yaw around Z axis
    
    # Draw a simple cube or other 3D object to represent your CanSat
    # For example, a wireframe cube:
    glColor3f(1.0, 1.0, 1.0)
    vertices = [
        (1, -1, -1), (1, 1, -1), (-1, 1, -1), (-1, -1, -1),
        (1, -1, 1), (1, 1, 1), (-1, -1, 1), (-1, 1, 1)
    ]
    edges = [
        (0, 1), (1, 2), (2, 3), (3, 0),
        (4, 5), (5, 7), (7, 6), (6, 4),
        (0, 4), (1, 5), (2, 7), (3, 6)
    ]
    
    glBegin(GL_LINES)
    for edge in edges:
        for vertex in edge:
            glVertex3fv(vertices[vertex])
    glEnd()
    
    # Draw the coordinate axes
    draw_axes()
    
    # Restore the matrix
    glPopMatrix()
    
    # Display text with current angles
    # This would require additional setup with PyGame fonts
    
    # Update the display
    pygame.display.flip()
    pygame.time.wait(10)

pygame.quit()
ser.close()
