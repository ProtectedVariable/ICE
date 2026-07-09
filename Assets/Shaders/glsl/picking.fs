#version 420 core

uniform int objectID;

out vec4 frag_color;

void main() {
    // Encode the 24-bit object id across R,G,B (decoded as R + G<<8 + B<<16). The old
    // code used integer division (/255) and had an operator-precedence bug (>> 16 / 255),
    // so ids >= 256 could not round-trip.
    frag_color = vec4(float(objectID & 0xFF) / 255.0,
                      float((objectID >> 8) & 0xFF) / 255.0,
                      float((objectID >> 16) & 0xFF) / 255.0,
                      1.0);
}