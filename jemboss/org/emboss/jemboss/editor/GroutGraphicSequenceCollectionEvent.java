package org.emboss.jemboss.editor;

/**
 * RendWindowContainerEvent.java
 *
 *
 * Created: Thu Mar 13 14:44:13 2003
 *
 * @author <a href="mailto:">Mr H. Morgan</a>
 * @version
 */
import java.beans.PropertyChangeEvent;

public class GroutGraphicSequenceCollectionEvent extends PropertyChangeEvent
{
  
  //private GroutGraphicSequenceCollection source;
  
	 /**
	 * Creates a new instance of GroutGraphicSequenceCollectionEvent 
	 * with source set
	 */
  public GroutGraphicSequenceCollectionEvent(Object source, 
																						 String propertyName, 
																						 Object oldValue, 
																						 Object newValue)
  {
    super(source, propertyName, oldValue, newValue);
    //this.source = (GroutGraphicSequenceCollection) source;
  }
  
}
