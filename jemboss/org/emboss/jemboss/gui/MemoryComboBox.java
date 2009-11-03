/********************************************************************
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public
*  License along with this library; if not, write to the
*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/


package org.emboss.jemboss.gui;

import java.awt.*;
import java.util.Vector;
import javax.swing.*;
import java.net.URL;

/**
*
* Extends <code>JComboBox<code> to add and store new
* elements
*
*/
public class MemoryComboBox extends JComboBox 
{

  public static final int MAX_MEM_LEN = 30;
  private Vector order;

  public MemoryComboBox(Vector v)
  {
    super(v);
    setEditable(true);

    order = new Vector();
    for(int i=0;i<v.size();i++)
      order.add(v.get(i));

    setFont(new Font(null,Font.PLAIN,12));
  }


  /**
  * 
  * Add an element to the combobox
  * @param item		element to add
  *
  */
  public void addURL(Object item)
  {
    removeItem(item);
    insertItemAt(item, 0);
    setSelectedItem(item);
    if(getItemCount() > MAX_MEM_LEN)
      removeItemAt(getItemCount()-1);

    order.add(item);
  }

  
  /**
  *
  * Ensure order is changed
  *
  */
  public void setLastIndex(Object anObject)
  {
    if(order.contains(anObject))
    {
      order.remove(anObject);
      order.trimToSize();
      order.add(anObject);
    }
  }
    
  
  /**
  *
  * Determine if a return page is stored
  * @return 	true if a return page available
  *
  */
  protected boolean isBackPage()
  {
    int currentIndex = order.indexOf(getSelectedItem());
    if(order.size() <= 1 || currentIndex < 1)
      return false;
    else
      return true;
  }


  /**
  *
  * Determine if a forward page is stored
  * @return     true if a forward page available
  *
  */
  protected boolean isForwardPage()
  {
    int currentIndex = order.indexOf(getSelectedItem());
    if(currentIndex < order.size()-1)
      return true;
    else 
      return false;
  }


  /**
  *
  * Determine if item is in the JComboBox
  * @param item		object to test existance of
  *
  */
  public boolean isItem(Object item)
  {
    int nitems = getItemCount();
    for(int i=0;i<nitems;i++)
      if(item.equals(getItemAt(i)))
        return true;
    return false;
  }


  public int getIndexOf(Object item)
  {
    return order.indexOf(item);
  }


  public URL getURLAt(int index)
  {
    return (URL)order.get(index);
  }

}

