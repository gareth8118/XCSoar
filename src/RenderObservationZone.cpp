#include "RenderObservationZone.hpp"
#include "Task/ObservationZones/LineSectorZone.hpp"
#include "Task/ObservationZones/FAISectorZone.hpp"
#include "Task/ObservationZones/KeyholeZone.hpp"
#include "Task/ObservationZones/BGAFixedCourseZone.hpp"
#include "Task/ObservationZones/BGAEnhancedOptionZone.hpp"
#include "Task/ObservationZones/CylinderZone.hpp"
#include "Screen/Graphics.hpp"
#include "WindowProjection.hpp"
#include "SettingsMap.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"

RenderObservationZone::RenderObservationZone(Canvas &_canvas,
                                             const Projection &_projection,
                                             const SETTINGS_MAP &_settings_map)
  :m_buffer(_canvas), m_proj(_projection),
   m_settings_map(_settings_map),
   layer(LAYER_SHADE),
   pen_boundary_current(Pen::SOLID, Layout::SmallScale(2), Graphics::TaskColor),
   pen_boundary_active(Pen::SOLID, Layout::SmallScale(1), Graphics::TaskColor),
   pen_boundary_inactive(Pen::SOLID, Layout::SmallScale(1), Color(127, 127, 127)),
   m_past(false),
   m_current(false)
{
}

bool 
RenderObservationZone::draw_style()
{
  if (layer == LAYER_SHADE) {
    if (m_past)
      return false;

#ifdef ENABLE_OPENGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Color color = Graphics::Colours[m_settings_map.iAirspaceColour[AATASK]];
    m_buffer.select(Brush(color.with_alpha(64)));
#else /* !OPENGL */

    m_buffer.mix_mask();

    // this color is used as the black bit
    m_buffer.set_text_color(Graphics::Colours[m_settings_map.
                                           iAirspaceColour[AATASK]]);
    // get brush, can be solid or a 1bpp bitmap
    m_buffer.select(Graphics::hAirspaceBrushes[m_settings_map.
                                            iAirspaceBrush[AATASK]]);
#endif /* !OPENGL */

    m_buffer.null_pen();
    
    return true;
  } else {
#ifdef ENABLE_OPENGL
    glDisable(GL_BLEND);
#else /* !OPENGL */
    m_buffer.mix_copy();
#endif /* !OPENGL */

    m_buffer.hollow_brush();
    if (layer == LAYER_ACTIVE && !m_past) {
      if (m_current)
        m_buffer.select(pen_boundary_current);
      else
        m_buffer.select(pen_boundary_active);
    } else {
      m_buffer.select(pen_boundary_inactive); 
    }
    return true;
  }
}

void 
RenderObservationZone::draw_two_lines() {
  m_buffer.two_lines(p_start, p_center, p_end);
}

void 
RenderObservationZone::draw_circle() {
  m_buffer.circle(p_center.x, p_center.y, p_radius);
}

void 
RenderObservationZone::draw_segment(const Angle start_radial, 
                                    const Angle end_radial) 
{
  m_buffer.segment(p_center.x, p_center.y, p_radius,
                   start_radial-m_proj.GetScreenAngle(), 
                   end_radial-m_proj.GetScreenAngle());
}

void 
RenderObservationZone::parms_oz(const CylinderZone& oz) 
{
  p_radius = m_proj.GeoToScreenDistance(oz.getRadius());
  p_center = m_proj.GeoToScreen(oz.get_location());
}

void 
RenderObservationZone::parms_sector(const SectorZone& oz) 
{
  parms_oz(oz);
  p_start = m_proj.GeoToScreen(oz.get_SectorStart());
  p_end = m_proj.GeoToScreen(oz.get_SectorEnd());
}

void 
RenderObservationZone::Visit(const FAISectorZone& oz) 
{
  if (!draw_style())
    return;

  parms_sector(oz);

  if (layer != LAYER_ACTIVE)
    draw_segment(oz.getStartRadial(), oz.getEndRadial());
  else
    draw_two_lines();

  m_buffer.mix_copy();
}

void 
RenderObservationZone::Visit(const KeyholeZone& oz) 
{
  if (!draw_style())
    return;

  parms_sector(oz);

  if (layer != LAYER_ACTIVE) {
    draw_segment(oz.getStartRadial(), oz.getEndRadial());
    p_radius = m_proj.GeoToScreenDistance(fixed(500));
    draw_circle();
  } else
    draw_two_lines();

  m_buffer.mix_copy();
}

void 
RenderObservationZone::Visit(const BGAFixedCourseZone& oz) 
{
  if (!draw_style())
    return;

  parms_sector(oz);

  if (layer != LAYER_ACTIVE) {
    draw_segment(oz.getStartRadial(), oz.getEndRadial());
    p_radius = m_proj.GeoToScreenDistance(fixed(500));
    draw_circle();
  } else
    draw_two_lines();

  m_buffer.mix_copy();
}

void 
RenderObservationZone::Visit(const BGAEnhancedOptionZone& oz) 
{
  if (!draw_style())
    return;

  parms_sector(oz);

  if (layer != LAYER_ACTIVE) {
    draw_segment(oz.getStartRadial(), oz.getEndRadial());
    p_radius = m_proj.GeoToScreenDistance(fixed(500));
    draw_circle();
  } else
    draw_two_lines();

  m_buffer.mix_copy();
}

void 
RenderObservationZone::Visit(const SectorZone& oz) 
{
  if (layer == LAYER_ACTIVE || !draw_style())
    return;

  parms_sector(oz);

  draw_segment(oz.getStartRadial(), oz.getEndRadial());
  draw_two_lines();

  m_buffer.mix_copy();
}

void 
RenderObservationZone::Visit(const LineSectorZone& oz) 
{
  if (!draw_style())
    return;

  parms_sector(oz);

  if (layer != LAYER_ACTIVE)
    draw_segment(oz.getStartRadial(), oz.getEndRadial());
  else
    draw_two_lines();

  m_buffer.mix_copy();
}

void 
RenderObservationZone::Visit(const CylinderZone& oz) 
{
  if (layer == LAYER_ACTIVE || !draw_style())
    return;

  parms_oz(oz);

  draw_circle();

  m_buffer.mix_copy();
}
