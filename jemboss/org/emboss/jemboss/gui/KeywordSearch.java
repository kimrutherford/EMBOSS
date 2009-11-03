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
                                                                                              
import javax.swing.*;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import java.awt.*;
import java.util.StringTokenizer;
import java.util.Arrays;
import java.util.Vector;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.StringReader;

import org.emboss.jemboss.JembossParams;

public class KeywordSearch implements HyperlinkListener
{

  private JTextPane searchHTML;
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  private JFrame f;

  public KeywordSearch(JTextField search, String woss,
                       JembossParams mysettings, boolean withSoap,
                       boolean andOperator)
  {
    String searchTxt = search.getText().trim().toLowerCase();
    String res = search(searchTxt,woss.toLowerCase(),
                        mysettings,withSoap,andOperator);
    showSearchResult(res,searchTxt,mysettings);
  }

  /**
  *
  * Search the text for matches of keywords
  * @param searchTxt  	keywords to search for
  * @param woss		wossname text to search
  * @param mysettings 	parameters
  * @param andOperator	true to use AND, false to use OR operator
  *
  */
  private String search(String searchTxt, String woss,
                        JembossParams mysettings, boolean withSoap,
                        boolean andOperator)
  {
    Vector vres = new Vector();
    StringBuffer res = new StringBuffer();
    boolean found = false;
    try
    {
      String stub;

      if(withSoap)
        stub = mysettings.getembURL();
      else
      {
        String embRoot = mysettings.getEmbossBin().trim();
        if(embRoot.endsWith("bin/"))
          embRoot = embRoot.substring(0,embRoot.length()-4);
        else
          embRoot = embRoot.substring(0,embRoot.length()-3);
          
        stub = "file://"+embRoot+"/share/EMBOSS/doc/html/emboss/apps/";
      }
 
      String line;
      String searching = getSearchText(searchTxt,andOperator);
 
      res.append("<h2><a name=\"SEARCH EMBOSS FOR "+searching+
                "\">EMBOSS SEARCH FOR '"+searching+"'</a></h2>");

      BufferedReader in = new BufferedReader(new StringReader(woss));
      while((line = in.readLine()) != null)
      {
        if(line.indexOf(":") > -1)
          continue;
        else
        {
          boolean match = false;

          if(andOperator)
            match = searchAND(line,searchTxt);
          else
            match = searchOR(line,searchTxt);

          if(match)
          {
            if(!found)
            {
              res.append("<table border cellpadding=4 bgcolor=\"#FFFFF0\">");
              res.append("<tr><th>Program name</th><th>Description</th></tr>");
            }

            found = true;
            String prog = line.substring(0,line.indexOf(" "));
            String progHTML = "<tr><td><a href=\""+stub+prog+
                              ".html\">"+prog+"</a></td><td>"+
                              line.substring(line.indexOf(" "))+"</td></tr>";
            if(!vres.contains(progHTML))
              vres.add(progHTML);
          }
        }            
      }

      if(found)
      {
        Object progOrder[] =  vres.toArray();
        Arrays.sort(progOrder);

        for(int i=0;i<progOrder.length; i++)
          res.append(progOrder[i]);
        res.append("</table>");
      }
      else
        res.append("<b>No matches found.</b>");
    }
    catch(IOException ioe){} 
    return res.toString();
  }

  /**
  *
  * Get the string representation for this search
  * e.g. given s1 s2, return s1 AND s2
  * @param searchTxt	search text
  * @param andOperator 	operator type (AND or OR)
  * @return string representation for this search
  *
  */
  private String getSearchText(String searchTxt, boolean andOperator)
  {
    if(searchTxt.indexOf(" ") == -1)
      return searchTxt;

    StringBuffer buff = new StringBuffer();
    StringTokenizer tok = new StringTokenizer(searchTxt," ");
    while(tok.hasMoreTokens())
    {
      buff.append(tok.nextToken());
      if(andOperator && tok.hasMoreTokens())
        buff.append(" AND ");
      else if(!andOperator && tok.hasMoreTokens())
        buff.append(" OR ");
    }
    return buff.toString();
  }

  /**
  *
  * Search a string for matches
  * @param line		string to search
  * @param searchTxt	keywords to search for
  * @return true if matched all keywords
  *
  */
  private boolean searchAND(String line, String searchTxt)
  {
    StringTokenizer tok = new StringTokenizer(searchTxt," ");
    while(tok.hasMoreTokens())
    {
      String token = tok.nextToken();
      if(line.indexOf(token) == -1)
        return false;
    }
    return true;
  }

  /**
  *
  * Search a string for matches
  * @param line         string to search
  * @param searchTxt    keywords to search for
  * @return true if matched at least one keyword
  *
  */
  private boolean searchOR(String line, String searchTxt)
  {
    StringTokenizer tok = new StringTokenizer(searchTxt," ");
    while(tok.hasMoreTokens())
    {
      String token = tok.nextToken();
      if(line.indexOf(token) > -1)
        return true;
    }
    return false;
  }

  /**
  *
  * Display search results 
  *
  */
  private void showSearchResult(String woss, String searchTxt,
                                JembossParams mysettings)
  {
    f = new JFrame("EMBOSS Keyword Search");
    Dimension d = f.getToolkit().getScreenSize();
    d = new Dimension((int)d.getWidth()/2,(int)d.getHeight()/2);
    f.setSize(d);
                                                                                            
    JTabbedPane tab = new JTabbedPane();
    searchHTML = new JTextPane();
    searchHTML.addHyperlinkListener(this);
    searchHTML.setEditable(false);
    searchHTML.setContentType("text/html");
    searchHTML.setText(woss);
    searchHTML.setCaretPosition(0);
                                                                                            
    JScrollPane jsp = new JScrollPane(searchHTML);
    jsp.setPreferredSize(d);
    tab.addTab("Search :: "+searchTxt,jsp);
    f.getContentPane().add(tab);
    new ResultsMenuBar(f,tab,null,mysettings);
    f.setVisible(true);
  }

  /**
  *
  * Method to handle hyper link events.
  * @param event        hyper link event
  *
  */
  public void hyperlinkUpdate(HyperlinkEvent event)
  {
    if(event.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
    {
      f.setCursor(cbusy);
      try
      {
        new Browser(event.getURL(), event.getDescription());
//      URL url = event.getURL();
//      searchHTML.setPage(url);
      }
      catch(IOException ioe)
      {
        
      }
      f.setCursor(cdone);
    }
//  else if(event.getEventType() == HyperlinkEvent.EventType.ENTERED)
//  else if(event.getEventType() == HyperlinkEvent.EventType.EXITED)
  }
}
