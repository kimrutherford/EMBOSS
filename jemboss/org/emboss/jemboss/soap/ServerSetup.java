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

package org.emboss.jemboss.soap;

import javax.swing.*;
import java.awt.*;
import java.util.*;

import java.awt.event.*;
import org.emboss.jemboss.gui.MemoryComboBox;
import org.emboss.jemboss.JembossParams;

/**
*
* Jemboss properties for the server setup
*
*/
public class ServerSetup extends JTabbedPane 
{
  
  /** public server URL   */
  private MemoryComboBox publicURL;
  /** private server URL  */
  private MemoryComboBox privateURL;
  /** public server name  */
  private MemoryComboBox publicName;
  /** private server name */
  private MemoryComboBox privateName;
  /** proxy name */
  private MemoryComboBox proxyName;
  /** proxy port */
  private MemoryComboBox proxyPort;
  /** proxy name for web browser */
  private MemoryComboBox proxyBrowserName;
  /** proxy port for web browser */
  private MemoryComboBox proxyBrowserPort;
  /** selected if user authetication required */
  private JCheckBox userAuth;
  /** selected if using proxy server */
  private JCheckBox useProxy;
  /** selected if using proxy server for web browser */
  private JCheckBox useBrowserProxy;
  /** selected if tfm help is run on the server */
  private JCheckBox callTFM;
  /** jemboss properties */
  private JembossParams myset; 

  /**
  *
  * @param mysettings   jemboss properties
  *
  */
  public ServerSetup(JembossParams mysettings)
  {
    this(mysettings,true);
  }

  /**
  *
  * @param mysettings	jemboss properties
  *
  */
  public ServerSetup(final JembossParams mysettings, boolean withSoap)
  {
    this.myset      = mysettings;
    GridLayout gl   = null;
    JPanel jpWest   = null;
    JPanel jpCenter = null;

    if(withSoap)
    {
      Vector PublicServerURL = new Vector();
      PublicServerURL.add(myset.getPublicSoapURL());

      Vector PrivateServerURL = new Vector();
      PrivateServerURL.add(myset.getPrivateSoapURL());

      Vector PublicServerName = new Vector();
      PublicServerName.add(myset.getPublicSoapService());

      Vector PrivateServerName = new Vector();
      PrivateServerName.add(myset.getPrivateSoapService());

      Vector proxyNameSettings = new Vector();
      proxyNameSettings.add(myset.getProxyHost());

      Vector proxyPortSettings = new Vector();
      proxyPortSettings.add(new Integer(myset.getProxyPortNum()));

      Vector proxyBrowserNameSettings = new Vector();
      proxyBrowserNameSettings.add(myset.getBrowserProxyHost());
    
      Vector proxyBrowserPortSettings = new Vector();
      proxyBrowserPortSettings.add(new Integer(myset.getBrowserProxyPort()));
//servers tabbed pane
      gl = new GridLayout(7,1,6,6);
 
      JPanel general  = new JPanel(new BorderLayout());   
      jpWest   = new JPanel(gl);
      jpCenter = new JPanel(gl);

//public server
      JLabel lab = new JLabel("Public Server");
      jpWest.add(lab);
      publicURL = new MemoryComboBox(PublicServerURL);
      jpCenter.add(publicURL);

      lab = new JLabel("Public Service Name");
      jpWest.add(lab);
      publicName = new MemoryComboBox(PublicServerName);
      jpCenter.add(publicName);

//separator
      jpWest.add(new JLabel(""));
      jpCenter.add(new JLabel(""));

//private server
      lab = new JLabel("Private Server");
      jpWest.add(lab);
      privateURL = new MemoryComboBox(PrivateServerURL);
      jpCenter.add(privateURL);    
  
      lab = new JLabel("Private Service Name ");
      jpWest.add(lab);
      privateName = new MemoryComboBox(PrivateServerName);
      jpCenter.add(privateName);

//separator
      jpWest.add(new JLabel(""));
      jpCenter.add(new JLabel(""));

//authentication 
      userAuth = new JCheckBox("User authentication required "+
                             "by private server",
                             myset.getUseAuth());

//separator
      jpWest.add(new JLabel(""));
      jpCenter.add(userAuth);

      general.add(jpWest,BorderLayout.WEST);
      general.add(jpCenter,BorderLayout.CENTER);

      addTab("Servers",general);


//proxy tabbed pane

      gl = new GridLayout(7,1,6,6);
      JPanel proxy = new JPanel(new BorderLayout());
      useProxy = new JCheckBox("Use proxy settings to connect to the server",
                              myset.getUseProxy());

      jpWest = new JPanel(gl);
      jpCenter = new JPanel(gl);
//separator
      jpWest.add(new JLabel(""));
      jpCenter.add(new JLabel(""));

      jpWest.add(new JLabel(""));
      jpCenter.add(useProxy);

// HTTPS proxys
      lab = new JLabel("Proxy ");
      jpWest.add(lab);
      proxyName = new MemoryComboBox(proxyNameSettings);
      jpCenter.add(proxyName);

      lab = new JLabel("Proxy Port");
      jpWest.add(lab);
      proxyPort = new MemoryComboBox(proxyPortSettings);
      jpCenter.add(proxyPort);

      proxyName.setEnabled(useProxy.isSelected());
      proxyPort.setEnabled(useProxy.isSelected());

      useProxy.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          proxyName.setEnabled(useProxy.isSelected());
          proxyPort.setEnabled(useProxy.isSelected());
          String settings[] = new String[1];
          settings[0] = new String("proxy.override=true");
          myset.updateJembossPropStrings(settings);
        }
      });

      jpWest.add(new JLabel(myset.proxyDescription()));
      jpCenter.add(new JLabel(""));

      jpWest.add(new JLabel(""));
      jpCenter.add(new JLabel(""));

      proxy.add(jpWest, BorderLayout.WEST);
      proxy.add(jpCenter, BorderLayout.CENTER);

      addTab("Proxies",proxy);

// HTTP proxys
      proxy = new JPanel(new BorderLayout());
      gl = new GridLayout(7,1,6,6);
      jpWest = new JPanel(gl);
      jpCenter = new JPanel(gl);

      useBrowserProxy = new JCheckBox("Use proxy settings for HTTP "+
                                    "browser connection",
                                    myset.isBrowserProxy());

      jpWest.add(new JLabel(""));
      jpCenter.add(new JLabel(""));

      jpWest.add(new JLabel(""));
      jpCenter.add(useBrowserProxy);

      lab = new JLabel("Proxy ");
      jpWest.add(lab);
      proxyBrowserName = new MemoryComboBox(proxyBrowserNameSettings);
        jpCenter.add(proxyBrowserName);

      lab = new JLabel("Proxy Port   ");
      jpWest.add(lab);
      proxyBrowserPort = new MemoryComboBox(proxyBrowserPortSettings);
      jpCenter.add(proxyBrowserPort);
    
      proxyBrowserName.setEnabled(useBrowserProxy.isSelected());
      proxyBrowserPort.setEnabled(useBrowserProxy.isSelected());
    
      useBrowserProxy.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          proxyBrowserName.setEnabled(useBrowserProxy.isSelected());
          proxyBrowserPort.setEnabled(useBrowserProxy.isSelected());
        }
      }); 

      callTFM = new JCheckBox("Get help page directly from the server");
      callTFM.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          useBrowserProxy.setEnabled(!callTFM.isSelected());
          proxyBrowserName.setEnabled(!callTFM.isSelected());
          proxyBrowserPort.setEnabled(!callTFM.isSelected());
        }
      });
      jpWest.add(new JLabel(""));
      jpCenter.add(callTFM);
      jpCenter.add(Box.createVerticalGlue());

      proxy.add(jpWest, BorderLayout.WEST);
      proxy.add(jpCenter, BorderLayout.CENTER);

      addTab("Browser Proxies",proxy);
    }

//client tabbed pane
    
    gl = new GridLayout(6,1,6,6);
    JPanel client = new JPanel(new BorderLayout());
    jpWest = new JPanel(gl);
    jpCenter = new JPanel(gl);

    jpWest.add(new JLabel("Java version  "));
    jpCenter.add(new JLabel(System.getProperty("java.version")));
     
    jpWest.add(new JLabel("Java home"));
    jpCenter.add(new JLabel(System.getProperty("java.home")));
     
    jpWest.add(new JLabel("User name "));
    jpCenter.add(new JLabel(System.getProperty("user.name")));
    
    jpWest.add(new JLabel("User home"));
    jpCenter.add(new JLabel(System.getProperty("user.home")));

    client.add(jpWest, BorderLayout.WEST);
    client.add(jpCenter, BorderLayout.CENTER);
                                                                               
    addTab("Client properties",client);

    if(!withSoap)
    {
      JPanel proxy = new JPanel(new BorderLayout());
      gl = new GridLayout(5,1,6,6);
      jpWest = new JPanel(gl);
      jpCenter = new JPanel(gl);
                                                                               
      useBrowserProxy = new JCheckBox("Use proxy settings for HTTP "+
                                    "browser connection",
                                    myset.isBrowserProxy());
                                                                               
      jpWest.add(new JLabel(""));
      jpCenter.add(new JLabel(""));
                                                                               
      jpWest.add(new JLabel(""));
      jpCenter.add(useBrowserProxy);
                                                                               
      JLabel lab = new JLabel("Proxy ");
      jpWest.add(lab);
      Vector proxyBrowserNameSettings = new Vector();
      if(System.getProperty("http.proxyHost") != null)
        proxyBrowserNameSettings.add(System.getProperty("http.proxyHost"));
      proxyBrowserName = new MemoryComboBox(proxyBrowserNameSettings);
        jpCenter.add(proxyBrowserName);
                                                                               
      lab = new JLabel("Proxy Port   ");
      jpWest.add(lab);
      Vector proxyBrowserPortSettings = new Vector();
      if(System.getProperty("http.proxyPort") != null)
        proxyBrowserPortSettings.add(System.getProperty("http.proxyPort"));
      proxyBrowserPort = new MemoryComboBox(proxyBrowserPortSettings);
      jpCenter.add(proxyBrowserPort);
                                                                               
      proxyBrowserName.setEnabled(useBrowserProxy.isSelected());
      proxyBrowserPort.setEnabled(useBrowserProxy.isSelected());
                                                                               
      useBrowserProxy.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          proxyBrowserName.setEnabled(useBrowserProxy.isSelected());
          proxyBrowserPort.setEnabled(useBrowserProxy.isSelected());
 
          if(useBrowserProxy.isSelected())
          {
            String settings[] = new String[3];
            settings[0] = new String("browserProxy.host="+
                             (String)proxyBrowserName.getSelectedItem());
            settings[1] = new String("browserProxy.port="+
                             (String)proxyBrowserPort.getSelectedItem());
            settings[2] = new String("browserProxy.use=true");
            mysettings.updateJembossPropStrings(settings);
          }
        }
      });
      proxy.add(jpWest, BorderLayout.WEST);
      proxy.add(jpCenter, BorderLayout.CENTER);

      addTab("Browser Proxies",proxy);
    }

  }

  /**
  *
  * Set the new settings in the server settings window
  *
  */
  public JembossParams setNewSettings()
  {
    String settings[] = new String[12];
    settings[0] = new String("server.public="+
                             (String)publicURL.getSelectedItem());
    settings[1] = new String("server.private="+
                             (String)privateURL.getSelectedItem());
    settings[2] = new String("service.public="+
                             (String)publicName.getSelectedItem());
    settings[3] = new String("service.private="+
                             (String)privateName.getSelectedItem());

    if(userAuth.isSelected())
      settings[4] = new String("user.auth=true");
    else
      settings[4] = new String("user.auth=false");

    settings[6] = new String("proxy.host="+
                             (String)proxyName.getSelectedItem());
    settings[7] = new String("proxy.port="+
               ((Integer)proxyPort.getSelectedItem()).toString());

    if(useProxy.isSelected())
      settings[5] = new String("proxy.use=true");
    else
      settings[5] = new String("proxy.use=false");

    settings[8] = new String("browserProxy.host="+
                             (String)proxyBrowserName.getSelectedItem());
    settings[9] = new String("browserProxy.port="+
               ((Integer)proxyBrowserPort.getSelectedItem()).toString());

    if(callTFM.isSelected())
      settings[10] = new String("tfm.use=true");
    else
      settings[10] = new String("tfm.use=false");

    if(useBrowserProxy.isSelected())
      settings[11] = new String("browserProxy.use=true");
    else
      settings[11] = new String("browserProxy.use=false");

    myset.updateJembossPropStrings(settings);

    return myset;
  }

}

