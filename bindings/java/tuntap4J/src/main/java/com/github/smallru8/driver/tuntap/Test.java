package com.github.smallru8.driver.tuntap;

import java.io.IOException;

/**
 * TunTap Test
 * Create a new tuntap device.
 */
public class Test 
{
    public static void main( String[] args ) throws InterruptedException
    {
        TunTap tt = new TunTap();
        tt.tuntap_up();
        try {
        	System.out.println("fd:"+tt.tuntap_get_fd());
        	System.out.println("press any key to continue...");
			System.in.read();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }
}
