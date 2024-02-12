/* GIMP RGBA C-Source image dump (mouse.c) */

#pragma once

static const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char	 pixel_data[12 * 19 * 4 + 1];
} mouse_pointer = {
  12, 19, 4,
  "\034\034\034\377\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\034\034\034\377\034\034"
  "\034\377\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\034\034\034\377\377\377\377\377\034\034\034\377"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\034\034\034\377\377\377\377\377\377\377\377\377\034\034\034\377\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\034\034\034\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\034\034\034\377\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\034\034\034\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\034\034\034\377\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\034\034\034\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\034\034\034\377\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\034\034\034\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\034\034\034\377\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\034\034\034\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\034\034\034\377\000\000\000\000\000\000\000\000\000\000\000\000"
  "\034\034\034\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\034\034\034\377\000\000\000\000\000\000\000\000\034\034\034\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\034\034"
  "\034\377\000\000\000\000\034\034\034\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\034\034\034\377"
  "\034\034\034\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\034\034\034\377\034\034\034\377"
  "\034\034\034\377\034\034\034\377\034\034\034\377\034\034\034\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\034\034\034\377\377\377\377\377"
  "\377\377\377\377\034\034\034\377\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\034\034\034\377\377\377\377\377\377\377\377\377"
  "\034\034\034\377\000\000\000\000\034\034\034\377\377\377\377\377\377\377"
  "\377\377\034\034\034\377\000\000\000\000\000\000\000\000\000\000\000\000"
  "\034\034\034\377\377\377\377\377\034\034\034\377\000\000\000\000\000\000"
  "\000\000\034\034\034\377\377\377\377\377\377\377\377\377\034\034\034\377"
  "\000\000\000\000\000\000\000\000\000\000\000\000\034\034\034\377\034\034"
  "\034\377\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\034\034\034\377\377\377\377\377\377\377\377\377\034\034\034\377\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\034\034\034\377\377\377"
  "\377\377\377\377\377\377\034\034\034\377\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  "\000\000\000\000\000\000\000\000\000\000\034\034\034\377\034\034\034\377"
  "\000\000\000\000\000\000\000\000\000\000\000\000",
};
