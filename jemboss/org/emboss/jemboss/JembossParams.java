/***************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss;

import java.util.*;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.net.InetAddress;


/**
*
* Contains all property information about the client
* and the server.
*
*/
public class JembossParams
{

/** denotes a server is OK             */
  static public final int SERVER_OK = 0;
/** denotes a server is giving errors  */
  static public final int SERVER_ERR = 1;
/** denotes a server is not responding */
  static public final int SERVER_DOWN = 2;

  // these are the things that could be set
  private boolean useHTTPSProxy = false;
  private String useHTTPSProxyName = "useHTTPSProxy";

  private boolean useProxy = false;
  private String useProxyName = "proxy.use";

  private String proxyHost = "wwwcache";
  private String proxyHostName = "proxy.host";

  private int proxyPortNum = 8080;
  private String proxyPortNumName = "proxy.port";

//browser proxy

  /** use a separate proxy for browsing the web                        */
  private boolean useBrowserProxy = false;
  /** property name for using separate proxy for browsing the web */
  private String useBrowserProxyName = "browserProxy.use";
  /** browser proxy host                   */
  private String browserProxyHost = "wwwcache";
  /** property name for browser proxy host  */
  private String browserProxyHostName = "browserProxy.host";
  /** browser proxy port                   */
  private int browserProxyPort = 8080;
  /** property name for browser proxy port */
  private String browserProxyPortName = "browserProxy.port";
 
  private boolean useTFM;
  private String useTFMName = "tfm.use";

  /** use proxy authentication                          */
  private boolean useProxyAuth = false;
  /** property name for using proxy authentication */ 
  private String useProxyAuthName = "proxy.auth";

  private String proxyAuthUser = "";
  private String proxyAuthUserName = "proxy.user";

  private String proxyAuthPasswd = "";
  private String proxyAuthPasswdName = "proxy.passwd";

  private boolean proxyOverride = false;
  private String proxyOverrideName = "proxy.override";

  /** use unix authentication to run applications on the server */
  private boolean useAuth = true;
  /** property name for using unix authentication               */
  private String useAuthName = "user.auth";

  /** public services URL                    */
  private String publicSoapURL = 
             "https://jemboss.hgmp.mrc.ac.uk:8443/axis/services";
  /** property name for public services URL  */
  private String publicSoapURLName = "server.public";

  /** private services URL                   */
  private String privateSoapURL = 
             "https://jemboss.hgmp.mrc.ac.uk:8443/axis/services";
  /** property name for private services URL */
  private String privateSoapURLName = "server.private";

  /** service name */
  private String soapService = "EmbossSoap";
  /** property name for service name */
  private String soapServiceName = "service.name";

  /** private service name                   */
  private String privateSoapService = "JembossAuthServer";
  /** property name for private service name */
  private String privateSoapServiceName = "service.private";

  /** public service name                    */
  private String publicSoapService = "JembossAuthServer";
  /** property name for public service name  */
  private String publicSoapServiceName = "service.public";

  //soap options
  private boolean debug = false;
  private String debugName = "jemboss.debug";

  /** batch mode support                   */
  private boolean hasBatchMode = true;
  /** property name for batch mode support */
  private String hasBatchModeName = "jemboss.hasbatchmode";
  /** interactive mode support                   */
  private boolean hasInteractiveMode = true;
  /** property name for interactive mode support */
  private String hasInteractiveModeName = "jemboss.hasinteractivemode";
  /** current mode for running an application    */
  private String currentMode = "interactive";
  /** property name for current mode             */
  private String currentModeName = "jemboss.mode";
  
  // server lists for redundancy
  private String serverPublicList = "";
  private String serverPublicListName = "server.publiclist";

  private String serverPrivateList = "";
  private String serverPrivateListName = "server.privatelist";
  
  // we don't remember these perhaps we should for captive systems
  private String serviceUserName = "";
  private String serviceUserNameName = "user.name";
  private char[] servicePasswd = null;
  /** services password */
  private byte[] servicePasswdByte = null;

  Properties jembossSettings;

  // Internal flags to help in the dynamic evaluation of properties
  private boolean useJavaProxy = false;
  private String javaProxyPort = "";
  private String javaProxyHost = "";
  private boolean useJavaNoProxy = false;
  private String javaNoProxy = "";
  private Vector javaNoProxyEntries;
  private int javaProxyPortNum = 8080;

  // structures for server redundancy
  private boolean publicServerFailOver = false;
  private boolean privateServerFailOver = false;
  private Hashtable serverStatusHash;
  private Vector publicServers;
  private Vector privateServers;

  /** Jemboss java server                   */
  private static boolean jembossServer = false;
  /** property name for Jemboss java server */
  private String jembossServerName = "jemboss.server";

  /** cygwin */
  private static String cygwin = null;
  /** property name for Jemboss java server */
  private String cygwinName = "cygwin";

  //EMBOSS directories
  /** plplot library location                            */
  private String plplot = "/usr/local/share/EMBOSS/";
  /** property name for plplot library location          */
  private String plplotName = "plplot";
  /** emboss data location                               */
  private String embossData = "/usr/local/share/EMBOSS/data/";
  /** property name for emboss data location             */
  private String embossDataName = "embossData";
  /** emboss binary location                             */
  private String embossBin = "/usr/local/bin/";
  /** property name for emboss binary location           */
  private String embossBinName = "embossBin";
  /** emboss path environment variable                   */
  private String embossPath = "/usr/bin/:/bin";
  /** property name for emboss path environment variable */
  private String embossPathName = "embossPath";
  /** emboss environment                                 */
  private String embossEnvironment = "";
  /** property name for emboss environment               */
  private String embossEnvironmentName = "embossEnvironment";
  /** acd file location                                  */
  private String acdDirToParse = "/usr/local/share/EMBOSS/acd/";
  /** property name for acd file location                */
  private String acdDirToParseName = "acdDirToParse";

  //EMBOSS Application pages
  /** documentation URL                      */
  private String embURL = "http://www.uk.embnet.org/Software/EMBOSS/Apps/";
  /** property name for documentation URL    */
  private String embossURL = "embossURL";

  // user properties
  /** user home directory                    */
  private String userHome = System.getProperty("user.home");
  /** property name for user home directory  */
  private String userHomeName = "user.home";
  
  /** standalone results directory           */
  private String resultsHome = System.getProperty("user.home")+ "/jemboss";
  private String resultsHomeName = "results.home";


/**
*
* Loads and holds the properties
*
*/
  public JembossParams() 
  {
    Properties defaults = new Properties();
    ClassLoader cl = this.getClass().getClassLoader();

    // initialize data structures
    serverStatusHash = new Hashtable();
    publicServers = new Vector();
    privateServers = new Vector();

    // initialize settings from table above
    defaults.put(userHomeName,userHome);
    defaults.put(resultsHomeName,resultsHome);
    defaults.put(embossURL,embURL);
    defaults.put(plplotName,plplot);
    defaults.put(embossDataName,embossData);
    defaults.put(embossBinName,embossBin);
    defaults.put(embossPathName,embossPath);
    defaults.put(embossEnvironmentName,embossEnvironment);
    defaults.put(acdDirToParseName,acdDirToParse);

    defaults.put(useBrowserProxyName, new Boolean(useBrowserProxy).toString());
    defaults.put(browserProxyHostName,browserProxyHost);
    defaults.put(browserProxyPortName,new Integer(browserProxyPort).toString());

    defaults.put(useTFMName,new Boolean(useTFM).toString());

    defaults.put(useProxyName, new Boolean(useProxy).toString());
    defaults.put(useHTTPSProxyName, new Boolean(useHTTPSProxy).toString());
    defaults.put(proxyHostName,proxyHost);
    defaults.put(proxyPortNumName, new Integer(proxyPortNum).toString());
    defaults.put(useProxyAuthName, new Boolean(useProxyAuth).toString());
    defaults.put(proxyAuthUserName, proxyAuthUser);
    defaults.put(proxyAuthPasswdName, proxyAuthPasswd);
    defaults.put(proxyOverrideName, new Boolean(proxyOverride).toString());
    defaults.put(useAuthName, new Boolean(useAuth).toString());
    defaults.put(publicSoapURLName, publicSoapURL);
    defaults.put(privateSoapURLName, privateSoapURL);
    defaults.put(privateSoapServiceName, privateSoapService);
    defaults.put(publicSoapServiceName, publicSoapService);
    defaults.put(debugName, new Boolean(debug).toString());
    defaults.put(hasBatchModeName, new Boolean(hasBatchMode).toString());
    defaults.put(hasInteractiveModeName, new Boolean(hasInteractiveMode).toString());
    defaults.put(currentModeName, currentMode);
    defaults.put(serverPublicListName, serverPublicList);
    defaults.put(serverPrivateListName, serverPrivateList);
    defaults.put(serviceUserNameName, serviceUserName);

    // load into real settings
    jembossSettings = new Properties(defaults);

    // try out of the classpath
    try
    {
      jembossSettings.load(cl.getResourceAsStream("resources/jemboss.properties"));
    } 
    catch (Exception e) 
    {
      if(debug)
        System.out.println("Didn't find properties file in classpath.");
    }

    // override with local system settings
    loadIn(System.getProperty("user.dir"));
    
    // override with local user settings
    loadIn(System.getProperty("user.home"));

    // update our settings
    updateSettingsFromProperties();

    if(System.getProperty("useHTTPSProxy") != null)
      if(System.getProperty("useHTTPSProxy").equalsIgnoreCase("true"))
        useHTTPSProxy=true;
    
    // set up for overrides
    javaNoProxyEntries = new Vector();
    if(System.getProperty("proxyPort") != null) 
    {
      if(System.getProperty("proxyHost") != null)
      {
	useJavaProxy = true;
        useProxy = useJavaProxy;
        useBrowserProxy = useJavaProxy;

	javaProxyPort = System.getProperty("proxyPort");
	javaProxyPortNum = Integer.parseInt(javaProxyPort);
	javaProxyHost = System.getProperty("proxyHost");

        browserProxyHost = javaProxyHost;
        browserProxyPort = javaProxyPortNum;
        
	if(System.getProperty("http.nonProxyHosts") != null) 
        {
	  useJavaNoProxy = true;
	  javaNoProxy = System.getProperty("http.nonProxyHosts");
	  StringTokenizer tok = new StringTokenizer(javaNoProxy,"|");
	  while (tok.hasMoreTokens()) 
          {
	    String toks = tok.nextToken() + "/";
	    javaNoProxyEntries.add(toks);
	  }
	}
      }
    }

  }

  /**
  *
  * Load a property from the jemboss.property file.
  * @param folder 	location of jemboss.property
  *
  */
  private void loadIn(String folder)
  {
    FileInputStream in = null;
    try
    {
      String fs = System.getProperty("file.separator");
      in = new FileInputStream(folder + fs + "jemboss.properties");
      jembossSettings.load(in);
    }
    catch (java.io.FileNotFoundException e)
    {
      in = null;
      if(debug)
        System.out.println("Can't find properties file in"+folder+"."+
                           " Using defaults.");
    }
    catch (java.io.IOException e)
    {
      if(debug)
        System.out.println("Can't read properties file. " +
                           "Using defaults.");
    }
    finally
    {
      if (in != null)
      {
        try { in.close(); } catch (java.io.IOException e) { }
        in = null;
      }
    }

  }

  /**
  *
  * Update the property settings for jembossSettings.
  *
  */
  protected void updateSettingsFromProperties()
  {

    try
    {
      String tmp;

      userHome = jembossSettings.getProperty(userHomeName);
      resultsHome = jembossSettings.getProperty(resultsHomeName);
      embURL = jembossSettings.getProperty(embossURL);
      plplot = jembossSettings.getProperty(plplotName);
      embossData = jembossSettings.getProperty(embossDataName);
      embossBin = jembossSettings.getProperty(embossBinName);
      embossPath = jembossSettings.getProperty(embossPathName);
      embossEnvironment = jembossSettings.getProperty(embossEnvironmentName);
      acdDirToParse = jembossSettings.getProperty(acdDirToParseName);
      tmp = jembossSettings.getProperty(jembossServerName);
      jembossServer = new Boolean(tmp).booleanValue();
         
      cygwin = jembossSettings.getProperty(cygwinName);
      
      tmp = jembossSettings.getProperty(useHTTPSProxyName);
      useHTTPSProxy = new Boolean(tmp).booleanValue();
      tmp = jembossSettings.getProperty(useProxyName);
      useProxy = new Boolean(tmp).booleanValue();
      proxyHost = jembossSettings.getProperty(proxyHostName);
      tmp = jembossSettings.getProperty(proxyPortNumName);
      proxyPortNum = Integer.parseInt(tmp);

      tmp = jembossSettings.getProperty(useBrowserProxyName);
      useBrowserProxy = new Boolean(tmp).booleanValue();
      browserProxyHost = jembossSettings.getProperty(browserProxyHostName);
      tmp = jembossSettings.getProperty(browserProxyPortName);
      browserProxyPort = Integer.parseInt(tmp);

      tmp = jembossSettings.getProperty(useTFMName);
      useTFM = new Boolean(tmp).booleanValue();

      tmp = jembossSettings.getProperty(useProxyAuthName);
      useProxyAuth = new Boolean(tmp).booleanValue();
      proxyAuthUser = jembossSettings.getProperty(proxyAuthUserName);
      proxyAuthPasswd = jembossSettings.getProperty(proxyAuthPasswdName);
      tmp = jembossSettings.getProperty(proxyOverrideName);
      proxyOverride = new Boolean(tmp).booleanValue();
      tmp = jembossSettings.getProperty(useAuthName);
      useAuth = new Boolean(tmp).booleanValue();
      publicSoapURL = jembossSettings.getProperty(publicSoapURLName);
      privateSoapURL = jembossSettings.getProperty(privateSoapURLName);
      soapService = jembossSettings.getProperty(soapServiceName);
      privateSoapService = jembossSettings.getProperty(privateSoapServiceName);
      publicSoapService = jembossSettings.getProperty(publicSoapServiceName);
      tmp = jembossSettings.getProperty(debugName);
      debug = new Boolean(tmp).booleanValue();
      tmp = jembossSettings.getProperty(hasBatchModeName);
      hasBatchMode = new Boolean(tmp).booleanValue();
      tmp = jembossSettings.getProperty(hasInteractiveModeName);
      hasInteractiveMode = new Boolean(tmp).booleanValue();
      currentMode = jembossSettings.getProperty(currentModeName);
      serverPublicList = jembossSettings.getProperty(serverPublicListName);
      serverPrivateList = jembossSettings.getProperty(serverPrivateListName);
//    serviceUserName = jembossSettings.getProperty(serviceUserNameName);
    } 
    catch (Exception e) {  }
  }

/**
*
* Initialize the server redundancy data. This is a separate
* method because the server info might not be initialized in
* the constructor, but may be imported later from the command line.
*
*/
  protected void setupServerRedundancy() 
  {
    if (!serverPublicList.equals("")) 
    {
      if(debug) 
	System.out.println("JembossParams: Redundant public servers\n  "
			   +serverPublicList);
  
      publicServerFailOver = true;
      StringTokenizer tok = new StringTokenizer(serverPublicList,"|");
      while (tok.hasMoreTokens()) 
      {
	String toks = tok.nextToken();
	publicServers.add(toks);
	if(debug) 
	  System.out.println(" Entry " + toks);
	
	serverStatusHash.put(toks, new Integer(SERVER_OK));
      }
    }

    if(!serverPrivateList.equals("")) 
    {
      if(debug) 
	System.out.println("JembossParams: Redundant private servers\n  "
			   +serverPrivateList);
      
      privateServerFailOver = true;
      StringTokenizer tok = new StringTokenizer(serverPrivateList,"|");
      while (tok.hasMoreTokens()) 
      {
	String toks = tok.nextToken();
	privateServers.add(toks);
	if(debug) 
	  System.out.println(" Entry " + toks);
	
	serverStatusHash.put(toks,new Integer(SERVER_OK));
      }
    }
  }


/**
*
* If using a proxy server
*
*/
  public boolean getUseProxy() 
  {
    return useProxy;
  }


/**
*
* If using an https proxy server
*
*/
  public boolean getUseHTTPSProxy()
  {
    return useHTTPSProxy;
  }


/**
*
* If using a proxy server for a given URL
* @param s 	the URL we wish to connect to
*
*/
  public boolean getUseProxy(String s) 
  {
    if(proxyOverride) 
      return useProxy;
    else 
    {
      if(!useJavaProxy) 
	return useProxy;
      else 
      {
	boolean jp = true;
	if (useJavaNoProxy) 
        {
	  int ip = javaNoProxyEntries.size();
	  for(int j = 0 ; j<ip ; ++j) 
	    if(s.indexOf((String)javaNoProxyEntries.get(j).toString()) != -1) 
	      jp = false;
	}
	return jp;
      }
    }
  }

/**
*
* The name of the proxy server
* @return 	name of the proxy host
*
*/
  public String getProxyHost() 
  {
    if (proxyOverride) 
      return proxyHost;
    else 
    {
      if(!useJavaProxy) 
	return proxyHost;
      else 
	return javaProxyHost;
    }
  }

/**
*
* The port the proxy server listens on
* @return 	proxy port number
*
*/
  public int getProxyPortNum() 
  {
    if(proxyOverride) 
      return proxyPortNum;
    else
    { 
      if(!useJavaProxy) 
	return proxyPortNum;
      else
	return javaProxyPortNum;
    }
  }

/**
*
* Determine if the a proxy server is being used for web browsing
* @return 	true if using a proxy server for the browser
*
*/
  public boolean isBrowserProxy()
  {
    return useBrowserProxy;
  }

/**
*
* Get the name of the proxy server for the browser
* @return 	the name of the proxy host
*
*/
  public String getBrowserProxyHost()
  {
    return browserProxyHost;
  }


/**
*
* The port number of the proxy server for the browser
* @return 	proxy port number 
*
*/
  public int getBrowserProxyPort()
  {
    return browserProxyPort;
  }


  public boolean isUseTFM() { return useTFM; }
  

/**
*
* If using authenticate with the proxy 
* @return	true if unix authentication used
*
*/
  public boolean getUseProxyAuth() 
  {
    return useProxyAuth;
  }

/**
*
* Username needed to use for the proxy server
*
*/
  public String getProxyAuthUser() 
  {
    return proxyAuthUser;
  }

/**
*
* Password needed to use for the proxy server
*
*/
  public String getProxyAuthPasswd() 
  {
    return proxyAuthPasswd;
  }

/**
*
* A description of the proxy settings
* @return 	a description of the proxy settings
*
*/
  public String proxyDescription() 
  {
    String pdesc = "";
    if (proxyOverride) 
    {
      if(useProxy)
      {
	String spnum = new Integer(proxyPortNum).toString();
	pdesc = "Current Settings: " + "Proxy Host: " + proxyHost + 
                                           " Proxy Port: " + spnum;
      } 
      else 
	pdesc = "No proxies, connecting direct.";
    } 
    else 
    {
      if (useJavaProxy) 
      {
	pdesc = "Settings Imported from Java: " + "Proxy Host: " + javaProxyHost
	                                      + " Proxy Port: " + javaProxyPort;
	if(useJavaNoProxy) 
	  pdesc = pdesc + "\nNo Proxy On: " + javaNoProxy;
      } 
      else
      {
	if(useProxy) 
        {
	  String spnum = new Integer(proxyPortNum).toString();
	  pdesc = "Current Settings: " + "Proxy Host: " + proxyHost + 
                                             " Proxy Port: " + spnum;
	} 
        else 
	  pdesc = "No proxies, connecting direct.";
      }
    }
    return pdesc;
  }

/**
*
* Whether the main service requires authentication
* @return 	true if the server is using unix authentication
*
*/
  public boolean getUseAuth() 
  {
    return useAuth;
  }

/**
*
* Returns the URL of the public soap server
* @return 	the public services URL
*
*/
  public String getPublicSoapURL() 
  {
    return publicSoapURL;
  }

/**
*
*  @return   	true if using a Jemboss server
*
*/
  public static boolean isJembossServer()
  {
    return jembossServer;
  }

/**
*
*  @return      true if using cygwin
*
*/
  public static boolean isCygwin()
  {
    if(cygwin == null || cygwin.equals(""))
      return false;
    else
      return true;
  }


/**
*
*  Get the cygwin root 
*  @return      cygwin root
*
*/
  public static String getCygwinRoot()
  {
    return cygwin;
  }



/**
*
* Get the location of plplot
* @return 	the location of plplot 
*
*/
  public String getPlplot()
  {
    return plplot;
  }

/**
*
* Get the user home directory
* @return	the user home directory
*
*/
  public String getUserHome()
  {
    return userHome;
  }

/**
*
* Set the user home directory property
* @param s	the user home directory
*
*/
  public void setUserHome(String s)
  {
    userHome = s;
  }


/**
*
* Get the standalone results directory
* @return       the results directory
*
*/
  public String getResultsHome()
  {
    String fs = System.getProperty("file.separator");

    if(!resultsHome.endsWith(fs))
      resultsHome = resultsHome + fs;
    return resultsHome;
  }

/**
*
* Set the standalone results directoryproperty
* @param s      the results directory
*
*/
  public void setResultsHome(String s)
  {
    resultsHome = s;
  }


/**
*
* @return 	the documentation URL 
*
*/
  public String getembURL()
  {
    return embURL;
  }

/**
*
* @return	the location of the emboss data
*
*/
  public String getEmbossData()
  {
    return embossData;
  }
  
/**
*
* @return	the location of the emboss binaries
*
*/
  public String getEmbossBin()
  {
    return embossBin;
  }

/**
*
* Get the path for emboss
* @return	the path for emboss
*
*/
  public String getEmbossPath()
  {
    return embossPath;
  }

/**
*
* Get the environment for emboss
* @return	the environment for emboss
*
*/
  public String getEmbossEnvironment()
  {
    embossEnvironment = embossEnvironment.trim();
    embossEnvironment = embossEnvironment.replace(':',' ');
    embossEnvironment = embossEnvironment.replace(',',' ');
    return embossEnvironment;
  }

/**
*
* Get the emboss environment as a String array
* @return 	the emboss environment as a String array
*
*/
  public String[] getEmbossEnvironmentArray(String[] envp)
  {
    embossEnvironment = embossEnvironment.trim();
    embossEnvironment = embossEnvironment.replace(',',' ');

    if(embossEnvironment.equals(""))
      return envp;

    StringTokenizer st = new StringTokenizer(embossEnvironment," ");
    int n=0;
    while(st.hasMoreTokens())
    {
      st.nextToken();
      n++;
    }
    
    int sizeEnvp = envp.length;
    String environ[] = new String[n+sizeEnvp];
    st = new StringTokenizer(embossEnvironment," ");
    for(int i=0;i<sizeEnvp;i++)
      environ[i] = envp[i];

    n=sizeEnvp;
    while(st.hasMoreTokens())
    {
      environ[n] = new String(st.nextToken()); 
      n++;
    }

    return environ;
  }

/**
*
* Acd file location
* @return 	acd file location
*
*/
  public String getAcdDirToParse()
  {
    return acdDirToParse;
  }

/**
*
* Set the URL of the public soap server
* @param s	URL of the public services
*
*/
  public void setPublicSoapURL(String s) 
  {
    publicSoapURL = s;
  }

/**
*
* Returns the URL of the private soap server
* @return 	URL of the private services
*
*/
  public String getPrivateSoapURL() 
  {
    return privateSoapURL;
  }

/**
*
* Set the URL of the private soap server
* @param s	URL of the private services
*
*/
  public void setPrivateSoapURL(String s) 
  {
    privateSoapURL = s;
  }

/**
*
* Return whether we have failover on the public server
*
*/
  public boolean getPublicServerFailover() 
  {
    return publicServerFailOver;
  }

/**
*
* Return whether we have failover on the private server
*
*/
  public boolean getPrivateServerFailover() 
  {
    return privateServerFailOver;
  }

/**
*
* Return a vector containing the list of public servers
*
*/
  public Vector getPublicServers() 
  {
    return publicServers;
  }

/**
*
* Return a vector containing the list of private servers
*
*/
  public Vector getPrivateServers() 
  {
    return privateServers;
  }

/**
*
* Mark a server as bad
*
*/
  public void setServerStatus(String server, int i) 
  {
    serverStatusHash.put(server, new Integer(i));
  }

/**
*
* Return the username needed for the remote service
* @return 	the username
*
*/
  public String getServiceUserName() 
  {
    return serviceUserName;
  }

/**
*
* Save the username needed for the remote service
* @param newUserName  	the username
*
*/
  public void setServiceUserName(String newUserName)
  {
    serviceUserName = newUserName;
  }

/**
*
* Return the password needed for the remote service
* @return 	password
*
*/
  public char[] getServicePasswd() 
  {
    return servicePasswd;
  }

/**
*
* Return the password needed for the remote service
* @return	password
*
*/
  public byte[] getServicePasswdByte()
  {
    return servicePasswdByte;
  }

/**
*
* Return the password as byte array
*
*/
  private static byte[] toByteArr(char ch[])
  {
    int len = ch.length;
    byte msb[] = new byte[len];

    for(int i=0;i<len;i++)
      msb[i] = (byte)(ch[i]);
    return msb;
  }


/**
*
* Save the password needed for the remote service
* @param newPasswd  	the username
*
*/
  public void setServicePasswd(char[] newPasswd) 
  {
    int csize = newPasswd.length;
    servicePasswd = new char[csize];

    for(int i=0;i<csize;i++)
      servicePasswd[i] = newPasswd[i];

    servicePasswdByte = toByteArr(newPasswd);
  }

/**
*
* Get the name of the soap service 
* @return 	soap service name
*
*/
  public String getSoapService() 
  {
    return soapService;
  }

/**
*
* Get the name of the private soap service 
* @return 	private service name 
*
*/
  public String getPrivateSoapService() 
  {
    return privateSoapService;
  }

/**
*
* Set the name of the private soap service
* @param s  	private service name
*
*/
  public void setPrivateSoapService(String s) 
  {
    privateSoapService = s;
  }

/**
*
* Get the name of the public soap service we're using
* @return 	public service name
*
*/
  public String getPublicSoapService() 
  {
    return publicSoapService;
  }

/**
*
* Set the name of the public soap service we're using
* @param s  	public service name
*
*/
  public void setPublicSoapService(String s) 
  {
    publicSoapService = s;
  }

/**
*
* A description of the server settings
* @return	description of the services
*
*/
  public String serverDescription() 
  {
    String serverdesc = "Current Settings:"
      +"\nPublic Server: "+publicSoapURL
      +"\nPrivate Server: "+privateSoapURL
      +"\nPublic SOAP service: "+publicSoapService
      +"\nPrivate SOAP service: "+privateSoapService;
    return serverdesc;
  }

/**
*
* Whether to show debugging information
*
*/
  public boolean getDebug() 
  {
    return debug;
  }

/**
*
* Whether this service supports batch mode
* @return	true if batch mode supported
*
*/
  public boolean getHasBatchMode() 
  {
    return hasBatchMode;
  }

/**
*
* Whether this service supports interactive mode
* @return     	true if interactive mode supported
*
*/
  public boolean getHasInteractiveMode() 
  {
    return hasInteractiveMode;
  }

/**
*
* The current mode (interactive or batch).
* @return 	a String containing the current mode
*
*/
  public String getCurrentMode() 
  {
    if(hasInteractiveMode) 
    {
      if (hasBatchMode) 
	return currentMode;
      else 
	return "interactive";
    } 
    else
    {
      if (hasBatchMode) 
	return "batch";
      else 
	return currentMode;
    }
  }

/**
*
* Set the current mode (interactive or batch). 
* @param newMode The new execution mode
*
*/
  public void setCurrentMode(String newMode) 
  {
    currentMode = newMode;
  }

/**
*
* Return the mode list as a vector, suitable for loading into
* a combobox.
* @return 	mode list
*
*/
  public Vector modeVector() 
  {
    Vector mv = new Vector();
    if (hasInteractiveMode) 
    {
      if (hasBatchMode) 
      {
	if (currentMode.equals("interactive")) 
        {
	  mv.add("interactive");
	  mv.add("batch");
	} 
        else if (currentMode.equals("batch")) 
        {
	  mv.add("batch");
	  mv.add("interactive");
	} 
        else 
        {
	  mv.add(currentMode);
	  mv.add("interactive");
	  mv.add("batch");
	}
      } 
      else 
	mv.add("interactive");
    } 
    else 
    {
      if(hasBatchMode) 
	mv.add("batch");
    }
    return mv;
  }

/**
*
* Update the properties structure. 
* This doesn't update the actual properties, just the Properties object
* so you must call updateSettingsFromProperties yoursefl
*
* @param  name   A String naming the property to be updated
* @param  newvalue  A String containing the new value of the property
*
*/
  public void updateJembossProperty(String name, String newvalue) 
  {
    if (jembossSettings.getProperty(name) != null) 
      jembossSettings.setProperty(name,newvalue);
  }

/**
*
* Parse a key=value string to update the properties structure
* @param entry String containing a key=value message
*
*/
  public void updateJembossPropString(String entry) 
  {
    int isep = entry.indexOf('=');
    if(isep != -1)
    {
      String pkey = entry.substring(0,isep);
      String pvalue = entry.substring(isep+1);
      updateJembossProperty(pkey, pvalue);
    }
  }

/**
*
* Parse an array of key=value strings to update the properties structure
* @param entries Array of Strings containing key=value messages
*
*/
  public void updateJembossPropStrings(String[] entries) 
  {
    for (int i=0; i < entries.length; ++i) 
      updateJembossPropString(entries[i]);
    
    updateSettingsFromProperties();
    setupServerRedundancy();
  }

/**
*
* Update properties from a Hashtable
* @param hash Hashtable containg properties
*
*/
  public void updateJembossPropHash(Hashtable hash) 
  {
    Enumeration enumer = hash.keys();
    while(enumer.hasMoreElements()) 
    {
      String thiskey = (String)enumer.nextElement().toString();
      String thisval = (String)hash.get(thiskey);
      updateJembossProperty(thiskey,thisval);
    }
    updateSettingsFromProperties();
  }

}

