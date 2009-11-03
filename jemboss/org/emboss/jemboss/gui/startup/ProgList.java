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


package org.emboss.jemboss.gui.startup;

import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.BevelBorder;
import java.awt.*;


/**
*
* Used on startup of Jemboss to retrieve the alphabetical list
* of programs and uses the programs groups to create the  
* menu structure based on the program type. This uses the output 
* of the EMBOSS program wossname.
*
*/
public class ProgList 
{

   /** array of the program names */
   private String allProgs[];
   /** array of program one line descriptions */
   private String allDescription[];
   /** number of programs */
   private int numProgs;
   /** number of JMenuItem created */
   private int nm;
   /** number of primary group programs */
   private int npG;
   /** used to create the program menus */
   private JMenuItem mItem[];
   /** font used for the menu items */
   private Font menuFont = new Font("SansSerif", Font.BOLD, 11);
   /** program group */
   private Hashtable programGroup = new Hashtable();

// private Font menuFont = new Font(null, Font.BOLD, 10);

   /**
   *
   * @param woss	output from wossname
   * @param menuBar	menu bar
   *
   */
   public ProgList(String woss, JMenuBar menuBar)
   {

     numProgs = 0;
     int numGroups = 0;
     int numMenuItems = 0;
// get alphabetic program listing
     String line;

//parse the output of wossname 
     try 
     {
       BufferedReader in;
       Vector allProgLines = new Vector();
       //while (numProgs == 0) 
       {
         in = new BufferedReader(new StringReader(woss));
         while((line = in.readLine()) != null)
         {
           numGroups++;
           while((line = in.readLine()) != null) 
           {
             line = line.trim();
             if(!line.equals(""))
             {
               numMenuItems++;
               //String progN = line.substring(0,line.indexOf(" ")+1);
               if(!allProgLines.contains(line))
               {
                 allProgLines.add(line);
                 numProgs++;
               }
             }
             else
               break;
            }
         }
         in.close();
       }
//load into string array
       allProgs = new String[numProgs];
       Enumeration e = allProgLines.elements();
       for(int i=0; e.hasMoreElements() ; i++)
         allProgs[i] =  (String)(e.nextElement());
     }
     catch (IOException e) 
     {
       System.out.println("Cannot read wossname string");
     }


//sort alphabetically
     java.util.Arrays.sort(allProgs,0,numProgs);
     allDescription = new String[numProgs];

     for(int i=0;i<numProgs;i++)
     {
       line = allProgs[i];
       int split = line.indexOf(" ");
       int len   = line.length();
       allProgs[i] = line.substring(0,split);
       line = line.substring(split+1,len).trim();
       allDescription[i] = new String(line);
     }


// get groups 

     HorizontalMenu primaryGroups[] = new HorizontalMenu[numGroups];
     JMenu secondaryGroups[] = new JMenu[numGroups];
     mItem = new JMenuItem[numMenuItems];

     npG=0;
     nm = 0;

     try 
     {
       String groups;
       String pg;
       String sg="";
       boolean exist;
       boolean sexist;
       int nsG=0;
       int index;
       int start=0;


       BufferedReader in = new BufferedReader(new StringReader(woss));

       while((groups = in.readLine()) != null)
       {
         groups = groups.trim();
         if(groups.equals(""))
           continue;

         index = groups.indexOf(":");  
         sexist = false;

         if(index > 0) 
         {
           pg = groups.substring(0,index);
           sg = groups.substring(index+1,groups.length());
           sexist = true;
         }
         else 
           pg = groups;

         exist = false;
         for(int j=0;j<npG;j++)
         {
           if(pg.equalsIgnoreCase(primaryGroups[j].getText()))
           {
             exist = true;
             index = j;
             break;
           }
         }

         if(!exist) 
         {
           primaryGroups[npG] = new HorizontalMenu(pg);
           primaryGroups[npG].setBorder(new BevelBorder(BevelBorder.RAISED));
           menuBar.add(primaryGroups[npG]);
           index = npG;
           npG++;
         }

         exist = false;
         if(sexist) 
         {
           for(int j=start;j<nsG;j++) 
           {
             if(sg.equalsIgnoreCase(secondaryGroups[j].getText())) 
             {
               exist = true;
               index = j;
               break;
             }
           }
           if(!exist) 
           {
             secondaryGroups[nsG] = new JMenu(sg);
             secondaryGroups[nsG].setFont(menuFont);
             primaryGroups[index].add(secondaryGroups[nsG]);
             index = nsG;
             nsG++;
           }
         }

         while((line = in.readLine()) != null) 
         {
           if(!line.equals("")) 
           {
             int split = line.indexOf(" ");
             int app=0;

             if(split < 0)
             {
               System.out.println("WARNING SKIPPING : " + line);
               continue;
             }
             String p = line.substring(0,split);
             for(int i=0;i<numProgs;i++) 
             {
               if(p.equalsIgnoreCase(allProgs[i]))
               {
                 programGroup.put(allProgs[i],groups);
                 app = i;
                 break;
               }
             }

//add the one line description to the menu 
             mItem[nm] = new JMenuItem(p + "   " + allDescription[app]);
             mItem[nm].setFont(menuFont);
             if(!sexist)
               primaryGroups[index].add(mItem[nm]);
             else
               secondaryGroups[index].add(mItem[nm]);
             nm++;
           } else 
             break;
         }
         start=nsG;
       }
       in.close();
     }
     catch (IOException ex) 
     {
       System.out.println("Cannot open EMBOSS acd file ");
     }

   }

  /**
  *
  * Returns the number of programs
  * @return number of programs
  *
  */
   public int getNumProgs() 
   {
     return numProgs;
   }

  /**
  *
  * Returns the array of all the program names
  * @return array of all the program names
  *
  */
   public String[] getProgsList() 
   {
     return allProgs;
   }

  /**
  *
  * Returns the array of the program descriptions
  * @return array of the program descriptions
  *
  */
   public String[] getProgDescription() 
   {
     return allDescription;
   }

  /**
  *
  * Returns the groups for the given application
  * @param app	program name
  * @return 	program group
  *
  */
  public String getProgramGroup(String app)
  {
    if(programGroup.containsKey(app))
      return (String)programGroup.get(app);
    else
      return null;
  }



  /**
  *
  * Writes to screen the program names
  *
  */
   public void writeList() 
   {
     for(int i=0;i<allProgs.length;i++)
       System.out.println(allProgs[i]);
   }

  /**
  *
  * Returns the program menu items 
  * @return program menu items
  *
  */
   public JMenuItem[] getMenuItems() 
   {
     return mItem;
   }

  /**
  *
  * Returns the number of program menu items
  * @return number of program menu items
  *
  */
   public int getNumberMenuItems() 
   {
     return nm;
   }

  /**
  *
  * Returns the number of primary menu groups
  * @return number of primary menu groups
  *
  */
   public int getNumPrimaryGroups() 
   {
     return npG;
   }
  
  /**
  *
  *  HorizontalMenu extends JMenu to produces horizontal 
  *  menus.
  *
  */
   class HorizontalMenu extends JMenu 
   {
     HorizontalMenu(String label) 
     {
       super(label);
       JPopupMenu pm = getPopupMenu();
       pm.setLayout(new BoxLayout(pm, BoxLayout.Y_AXIS));
       setFont(menuFont);
       setMinimumSize(getPreferredSize());
     }
        
     public void setPopupMenuVisible(boolean b) 
     {
       boolean isVisible = isPopupMenuVisible();
       if (b != isVisible) {
         if ((b==true) && isShowing()) {
           // Set location of popupMenu (pulldown or pullright)
           // Perhaps this should be dictated by L&F
           int x = 0;
           int y = 0;
           Container parent = getParent();
           if(parent instanceof JPopupMenu) 
           {
             x = 0;
             y = getHeight();
           } 
           else 
           {
             x = getWidth();
             y = 0;
           }
           getPopupMenu().show(this, x, y);
         } 
         else 
         {
           getPopupMenu().setVisible(false);
         }
       }
     }
   }

}

