package org.emboss.jemboss.editor;

/**
 * GroutAlignJFrameEvent.java
 *
 *
 * Created: Thu Mar 13 14:44:13 2003
 *
 * @author <a href="mailto:">Mr H. Morgan</a>
 * @version
 */
import java.util.EventObject;
import java.beans.PropertyChangeEvent;

public class GroutAlignJFrameEvent extends PropertyChangeEvent
{
  
  private GroutAlignJFrame source;
  
  /** Creates a new instance of GroutAlignJFrameEvent with source set */
  public GroutAlignJFrameEvent(Object source, String propertyName, 
																Object oldValue, Object newValue)
  {
    super(source, propertyName, oldValue, newValue);
    this.source = (GroutAlignJFrame) source;
  }
  
}
