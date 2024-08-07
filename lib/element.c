/* Dia -- an diagram creation/manipulation program
 * Copyright (C) 1998 Alexander Larsson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element
 * @title: Element
 *
 * The Element object type is a rectangular box that has
 * non-connectable handles on its corners and on the midsts of the
 * edges.  It also has connectionpoints in the same places as the
 * handles as well as a mainpoint in the middle.
 */

#include "config.h"

#include <glib/gi18n-lib.h>

#include <stdio.h>
#include <math.h>
#include <string.h> /* memcpy() */

#include "element.h"
#include "properties.h"


#ifdef G_OS_WIN32
/* defined in header */
#else
PropNumData width_range = { -G_MAXFLOAT, G_MAXFLOAT, 0.1};
#endif


/**
 * element_update_boundingbox:
 * @elem: An object to update bounding box on.
 *
 * Update the boundingbox information for this element.
 *
 * Since: dawn-of-time
 */
void
element_update_boundingbox (Element *elem)
{
  DiaRectangle bb;
  Point *corner;
  ElementBBExtras *extra = &elem->extra_spacing;

  g_return_if_fail (elem != NULL);

  corner = &elem->corner;
  bb.left = corner->x;
  bb.right = corner->x + elem->width;
  bb.top = corner->y;
  bb.bottom = corner->y + elem->height;

  rectangle_bbox (&bb, extra, &elem->object.bounding_box);
}


/**
 * element_update_connections_rectangle:
 * @elem: The element to update
 * @cps: The list of connectionpoints to update, in the order described.
 *       Usually, this will be the same list as elem->connectionpoints.
 *
 * Update the 9 connections of this element to form a rectangle and
 * point in the center.
 * The connections go left-to-right, first top row, then middle row, then
 * bottom row, then center.  Do not blindly use this in old objects where
 * the order is different, as it will mess with the saved files.  If an
 * object uses element_update_handles, it can use this.
 */
void
element_update_connections_rectangle (Element         *elem,
                                      ConnectionPoint *cps)
{
  cps[0].pos = elem->corner;
  cps[1].pos.x = elem->corner.x + elem->width / 2.0;
  cps[1].pos.y = elem->corner.y;
  cps[2].pos.x = elem->corner.x + elem->width;
  cps[2].pos.y = elem->corner.y;
  cps[3].pos.x = elem->corner.x;
  cps[3].pos.y = elem->corner.y + elem->height / 2.0;
  cps[4].pos.x = elem->corner.x + elem->width;
  cps[4].pos.y = elem->corner.y + elem->height / 2.0;
  cps[5].pos.x = elem->corner.x;
  cps[5].pos.y = elem->corner.y + elem->height;
  cps[6].pos.x = elem->corner.x + elem->width / 2.0;
  cps[6].pos.y = elem->corner.y + elem->height;
  cps[7].pos.x = elem->corner.x + elem->width;
  cps[7].pos.y = elem->corner.y + elem->height;
  g_assert(elem->object.num_connections >= 9);
  cps[8].pos.x = elem->corner.x + elem->width / 2.0;
  cps[8].pos.y = elem->corner.y + elem->height / 2.0;

  cps[0].directions = DIR_NORTH|DIR_WEST;
  cps[1].directions = DIR_NORTH;
  cps[2].directions = DIR_NORTH|DIR_EAST;
  cps[3].directions = DIR_WEST;
  cps[4].directions = DIR_EAST;
  cps[5].directions = DIR_SOUTH|DIR_WEST;
  cps[6].directions = DIR_SOUTH;
  cps[7].directions = DIR_SOUTH|DIR_EAST;
  cps[8].directions = DIR_ALL;
}


/**
 * element_update_connections_directions:
 * @elem: the #Element
 * @cps: list of #ConnectionPoint
 *
 * More elaborate variant to calculate connection point directions
 *
 * It works for any number of connection points.
 * It calculates them based on qudrants, so works best for symmetric elements.
 *
 * Since: dawn-of-time
 */
void
element_update_connections_directions (Element         *elem,
                                       ConnectionPoint *cps)
{
  Point center = { elem->corner.x + elem->width / 2, elem->corner.y + elem->height / 2.0 };
  int i;

  for (i = 0; i < elem->object.num_connections; ++i) {
    cps[i].directions = DIR_NONE;
    if (cps[i].pos.x > center.x)
      cps[i].directions |= DIR_EAST;
    else if (cps[i].pos.x < center.x)
      cps[i].directions |= DIR_WEST;
    if (cps[i].pos.y > center.y)
      cps[i].directions |= DIR_SOUTH;
    else if (cps[i].pos.y < center.y)
      cps[i].directions |= DIR_NORTH;
    if (cps[i].flags == CP_FLAGS_MAIN)
      cps[i].directions |= DIR_ALL;
  }
}


/**
 * element_update_handles:
 * @elem: An element to update.
 *
 * Update the corner and edge handles of an element to reflect its position
 * and size.
 */
void
element_update_handles (Element *elem)
{
  Point *corner = &elem->corner;

  elem->resize_handles[0].id = HANDLE_RESIZE_NW;
  elem->resize_handles[0].pos.x = corner->x;
  elem->resize_handles[0].pos.y = corner->y;

  elem->resize_handles[1].id = HANDLE_RESIZE_N;
  elem->resize_handles[1].pos.x = corner->x + elem->width/2.0;
  elem->resize_handles[1].pos.y = corner->y;

  elem->resize_handles[2].id = HANDLE_RESIZE_NE;
  elem->resize_handles[2].pos.x = corner->x + elem->width;
  elem->resize_handles[2].pos.y = corner->y;

  elem->resize_handles[3].id = HANDLE_RESIZE_W;
  elem->resize_handles[3].pos.x = corner->x;
  elem->resize_handles[3].pos.y = corner->y + elem->height/2.0;

  elem->resize_handles[4].id = HANDLE_RESIZE_E;
  elem->resize_handles[4].pos.x = corner->x + elem->width;
  elem->resize_handles[4].pos.y = corner->y + elem->height/2.0;

  elem->resize_handles[5].id = HANDLE_RESIZE_SW;
  elem->resize_handles[5].pos.x = corner->x;
  elem->resize_handles[5].pos.y = corner->y + elem->height;

  elem->resize_handles[6].id = HANDLE_RESIZE_S;
  elem->resize_handles[6].pos.x = corner->x + elem->width/2.0;
  elem->resize_handles[6].pos.y = corner->y + elem->height;

  elem->resize_handles[7].id = HANDLE_RESIZE_SE;
  elem->resize_handles[7].pos.x = corner->x + elem->width;
  elem->resize_handles[7].pos.y = corner->y + elem->height;
}


/**
 * element_move_handle:
 * @elem: The element whose handle is being moved.
 * @id: The id of the handle.
 * @to: Where it's being moved to.
 * @cp: Ignored
 * @reason: What is causing this handle to be moved (creation, movement..)
 * @modifiers: Any modifier keys (shift, control...) that the user is
 *             pressing.
 *
 * Handle the moving of one of the elements handles.
 * This function is suitable for use as the move_handle object operation.
 *
 * Returns: Undo information for this change.
 */
DiaObjectChange *
element_move_handle (Element          *elem,
                     HandleId          id,
                     Point            *to,
                     ConnectionPoint  *cp,
                     HandleMoveReason  reason,
                     ModifierKeys      modifiers)
{
  Point p;
  Point *corner;

  g_return_val_if_fail (id >= HANDLE_RESIZE_NW, NULL);
  g_return_val_if_fail (id <= HANDLE_RESIZE_SE, NULL);

  corner = &elem->corner;

  p = *to;
  point_sub (&p, &elem->corner);

  switch (id) {
    case HANDLE_RESIZE_NW:
      if ( to->x < (corner->x+elem->width)) {
        corner->x += p.x;
        elem->width -= p.x;
      }
      if ( to->y < (corner->y+elem->height)) {
        corner->y += p.y;
        elem->height -= p.y;
      }
      break;
    case HANDLE_RESIZE_N:
      if ( to->y < (corner->y+elem->height)) {
        corner->y += p.y;
        elem->height -= p.y;
      }
      break;
    case HANDLE_RESIZE_NE:
      if (p.x>0.0)
        elem->width = p.x;
      if ( to->y < (corner->y+elem->height)) {
        corner->y += p.y;
        elem->height -= p.y;
      }
      break;
    case HANDLE_RESIZE_W:
      if ( to->x < (corner->x+elem->width)) {
        corner->x += p.x;
        elem->width -= p.x;
      }
      break;
    case HANDLE_RESIZE_E:
      if (p.x>0.0) {
        elem->width = p.x;
      }
      break;
    case HANDLE_RESIZE_SW:
      if ( to->x < (corner->x+elem->width)) {
        corner->x += p.x;
        elem->width -= p.x;
      }
      if (p.y>0.0) {
        elem->height = p.y;
      }
      break;
    case HANDLE_RESIZE_S:
      if (p.y>0.0) {
        elem->height = p.y;
      }
      break;
    case HANDLE_RESIZE_SE:
      if (p.x>0.0) {
        elem->width = p.x;
      }
      if (p.y>0.0) {
        elem->height = p.y;
      }
      break;
    case HANDLE_MOVE_STARTPOINT:
    case HANDLE_MOVE_ENDPOINT:
    case HANDLE_CUSTOM1:
    case HANDLE_CUSTOM2:
    case HANDLE_CUSTOM3:
    case HANDLE_CUSTOM4:
    case HANDLE_CUSTOM5:
    case HANDLE_CUSTOM6:
    case HANDLE_CUSTOM7:
    case HANDLE_CUSTOM8:
    case HANDLE_CUSTOM9:
    default:
      g_warning ("element_move_handle() called with wrong handle-id\n");
  }

  return NULL;
}


/**
 * element_move_handle_aspect:
 * @elem: The element to update on
 * @id: The id of the handle being moved
 * @to: Where the handle is being moved to
 * @aspect_ratio: The aspect ratio (width:height) to obey.
 *                The aspect ratio must not be 0.
 *
 * Move the handle of an element restricted to a certain aspect ration.
 */
void
element_move_handle_aspect (Element  *elem,
                            HandleId  id,
                            Point    *to,
                            real      aspect_ratio)
{
  Point p;
  Point *corner;
  real width, height;
  real new_width, new_height;
  real move_x=0;
  real move_y=0;

  g_return_if_fail (id >= HANDLE_RESIZE_NW);
  g_return_if_fail (id <= HANDLE_RESIZE_SE);

  corner = &elem->corner;

  p = *to;
  point_sub (&p, &elem->corner);

  width = elem->width;
  height = elem->height;

  new_width = 0.0;
  new_height = 0.0;


  switch (id) {
    case HANDLE_RESIZE_NW:
      new_width = width - p.x;
      new_height = height - p.y;
      move_x = 1.0;
      move_y = 1.0;
      break;
    case HANDLE_RESIZE_N:
      new_height = height - p.y;
      move_y = 1.0;
      move_x = 0.5;
      break;
    case HANDLE_RESIZE_NE:
      new_width = p.x;
      new_height = height - p.y;
      move_x = 0.0;
      move_y = 1.0;
      break;
    case HANDLE_RESIZE_W:
      new_width = width - p.x;
      move_x = 1.0;
      move_y = 0.5;
      break;
    case HANDLE_RESIZE_E:
      new_width = p.x;
      move_x = 0.0;
      move_y = 0.5;
      break;
    case HANDLE_RESIZE_SW:
      new_width = width - p.x;
      new_height = p.y;
      move_x = 1.0;
      move_y = 0.0;
      break;
    case HANDLE_RESIZE_S:
      new_height = p.y;
      move_x = 0.5;
      move_y = 0.0;
      break;
    case HANDLE_RESIZE_SE:
      new_width = p.x;
      new_height = p.y;
      move_x = 0.0;
      move_y = 0.0;
      break;
    case HANDLE_MOVE_STARTPOINT:
    case HANDLE_MOVE_ENDPOINT:
    case HANDLE_CUSTOM1:
    case HANDLE_CUSTOM2:
    case HANDLE_CUSTOM3:
    case HANDLE_CUSTOM4:
    case HANDLE_CUSTOM5:
    case HANDLE_CUSTOM6:
    case HANDLE_CUSTOM7:
    case HANDLE_CUSTOM8:
    case HANDLE_CUSTOM9:
    default:
      g_warning ("element_move_handle() called with wrong handle-id\n");
  }

  /* Which of the two versions to use: */
  if (new_width > new_height*aspect_ratio) {
    new_height = new_width/aspect_ratio;
  } else {
    new_width = new_height*aspect_ratio;
  }

  if ( (new_width<0.0) || (new_height<0.0)) {
    new_width = 0.0;
    new_height = 0.0;
  }

  corner->x -= (new_width - width)*move_x;
  corner->y -= (new_height - height)*move_y;

  elem->width  = new_width;
  elem->height = new_height;
}


/**
 * element_init:
 * @elem: The element to initialize. This function will call
 *        object_init() on the element.
 * @num_handles: The number of handles to set up (>= 8). The handles
 *               will be initialized by this function.
 * @num_connections: The number of connectionpoints to set up (>= 9).
 *                   The connectionpoints will _not_ be
 *                   initialized by this function.
 *
 * Initialization function for element objects.
 *
 * An element must have at least 8 handles and 9 connectionpoints.
 */
void
element_init (Element *elem, int num_handles, int num_connections)
{
  DiaObject *obj;
  int i;

  obj = &elem->object;

  g_return_if_fail (num_handles >= 8);

  object_init (obj, num_handles, num_connections);

  for (i = 0; i < 8;i ++) {
    obj->handles[i] = &elem->resize_handles[i];
    obj->handles[i]->connect_type = HANDLE_NONCONNECTABLE;
    obj->handles[i]->connected_to = NULL;
    obj->handles[i]->type = HANDLE_MAJOR_CONTROL;
  }
}


/**
 * element_copy:
 * @from: An element to copy from.
 * @to: An element (already allocated) to copy to.
 *
 * Copy an element, initializing the handles.
 *
 * This function will in turn copy the underlying object.
 */
void
element_copy (Element *from, Element *to)
{
  DiaObject *toobj, *fromobj;
  int i;

  fromobj = &from->object;
  toobj = &to->object;

  object_copy (fromobj, toobj);

  to->corner = from->corner;
  to->width = from->width;
  to->height = from->height;

  for (i = 0; i < 8; i++) {
    to->resize_handles[i] = from->resize_handles[i];
    to->resize_handles[i].connected_to = NULL;
    toobj->handles[i] = &to->resize_handles[i];
  }

  memcpy (&to->extra_spacing,
          &from->extra_spacing,
          sizeof (to->extra_spacing));
}


/**
 * element_destroy:
 * @elem: The element to destroy. It will _not_ be deallocated
 *        by this call, but will not be valid afterwards.
 *
 * Destroy an elements private information.
 *
 * This function will in turn call object_destroy().
 */
void
element_destroy (Element *elem)
{
  object_destroy (&elem->object);
}


/**
 * element_save:
 * @elem: the #Element to save
 * @obj_node: the #ObjectNode to save to
 * @ctx: the current #DiaContent
 *
 * Save the element-specific parts of this element to XML.
 *
 * Since: dawn-of-time
 */
void
element_save (Element *elem, ObjectNode obj_node, DiaContext *ctx)
{
  object_save (DIA_OBJECT (elem), obj_node, ctx);

  data_add_point (new_attribute (obj_node, "elem_corner"),
                  &elem->corner, ctx);
  data_add_real (new_attribute (obj_node, "elem_width"),
                 elem->width, ctx);
  data_add_real (new_attribute (obj_node, "elem_height"),
                 elem->height, ctx);
}


void
element_load (Element *elem, ObjectNode obj_node, DiaContext *ctx)
{
  AttributeNode attr;

  object_load (&elem->object, obj_node, ctx);

  elem->corner.x = 0.0;
  elem->corner.y = 0.0;
  attr = object_find_attribute (obj_node, "elem_corner");
  if (attr != NULL) {
    data_point (attribute_first_data (attr), &elem->corner, ctx);
  }

  elem->width = 1.0;
  attr = object_find_attribute (obj_node, "elem_width");
  if (attr != NULL) {
    elem->width = data_real (attribute_first_data (attr), ctx);
  }

  elem->height = 1.0;
  attr = object_find_attribute (obj_node, "elem_height");
  if (attr != NULL) {
    elem->height = data_real (attribute_first_data (attr), ctx);
  }
}


struct _DiaElementObjectChange {
  DiaObjectChange object_change;

  Element *element;
  Point    corner;
  double   width;
  double   height;
};


DIA_DEFINE_OBJECT_CHANGE (DiaElementObjectChange, dia_element_object_change)


static void
dia_element_object_change_free (DiaObjectChange *self)
{

}


static void
_element_change_swap (DiaElementObjectChange *ec, DiaObject *obj)
{
  Element *elem = ec->element;
  Point tmppt;
  real  tmp;

  g_return_if_fail (!obj || obj == &(ec->element->object));

  tmppt = ec->corner; ec->corner = elem->object.position; elem->object.position = tmppt;
  tmp = ec->width; ec->width = elem->width; elem->width = tmp;
  tmp = ec->height; ec->height = elem->height; elem->height = tmp;
}


static void
dia_element_object_change_revert (DiaObjectChange *self, DiaObject *obj)
{
  _element_change_swap (DIA_ELEMENT_OBJECT_CHANGE (self), obj);
}


static void
dia_element_object_change_apply (DiaObjectChange *self, DiaObject *obj)
{
  _element_change_swap (DIA_ELEMENT_OBJECT_CHANGE (self), obj);
}


DiaObjectChange *
element_change_new (const Point *corner,
                    double       width,
                    double       height,
                    Element     *elem)
{
  DiaElementObjectChange *ec = dia_object_change_new (DIA_TYPE_ELEMENT_OBJECT_CHANGE);

  ec->element = elem;
  ec->corner = elem->corner;
  ec->width = elem->width;
  ec->height = elem->height;

  return DIA_OBJECT_CHANGE (ec);
}


void
element_get_poly (const Element *elem, real angle, Point corners[4])
{
  corners[0] = elem->corner;
  corners[1] = corners[0];
  corners[1].x += elem->width;
  corners[2] = corners[1];
  corners[2].y += elem->height;
  corners[3] = corners[2];
  corners[3].x -= elem->width;

  if (angle != 0) {
    real cx = elem->corner.x + elem->width / 2.0;
    real cy = elem->corner.y + elem->height / 2.0;
    DiaMatrix m = { 1.0, 0.0, 0.0, 1.0, cx, cy };
    DiaMatrix t = { 1.0, 0.0, 0.0, 1.0, -cx, -cy };
    int i;

    dia_matrix_set_angle_and_scales (&m, G_PI*angle/180, 1.0, 1.0);
    dia_matrix_multiply (&m, &t, &m);
    for (i = 0; i < 4; ++i) {
      transform_point (&corners[i], &m);
    }
  }
}
