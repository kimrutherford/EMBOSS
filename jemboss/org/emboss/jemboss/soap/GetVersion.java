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

import javax.xml.namespace.QName;

import org.apache.axis.client.Call;
import org.emboss.jemboss.JembossParams;

/**
 * 
 * Get the version of EMBOSS programs used by the Jemboss server connected
 * 
 */

public class GetVersion {

	private static String version = null;

	/**
	 * returns the complete version string of the EMBOSS installation,
	 * which is used by the Jemboss server current Jemboss client connected
	 * 
	 * @param mysettings    jemboss properties
	 * 
	 */
	public static String getVersionComplete(JembossParams mysettings) {

		if (version != null)
			return version;

		if (!JembossParams.isJembossServer())
			return null;
		try {
			String endpoint = mysettings.getPublicSoapURL();
			org.apache.axis.client.Service serv = new org.apache.axis.client.Service();

			Call call = (Call) serv.createCall();
			call.setTargetEndpointAddress(new java.net.URL(endpoint));
			call.setOperationName(new QName(mysettings.getPublicSoapService(),
					"version"));
			call.setReturnType(org.apache.axis.Constants.SOAP_STRING);
			version = (String) call.invoke(new Object[] {});
		} catch (Exception jse) {
			jse.printStackTrace();
		}
		return version;
	}
	
	
	/**
	 * returns the first two digits of the EMBOSS version number
	 * @param mysettings
	 * @return version of EMBOSS programs such as 5.0, or 6.1
	 */
	public static String getVersion(JembossParams mysettings){
		getVersionComplete(mysettings);
		return version.split("\\.")[0]+"."+version.split("\\.")[1];        
	}

}
