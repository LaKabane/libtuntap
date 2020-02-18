package com.github.smallru8.driver.tuntap;

import java.io.File;
import java.util.Properties;

public class TunTap {
	public native void tuntap_init();
	public native int tuntap_version();
	public native void tuntap_destroy();
	public native void	tuntap_release();
	public native int tuntap_start(int mode,int unit);//0x0001 257
	public native String tuntap_get_ifname();
	public native int tuntap_set_ifname(String ifname);
	public native String tuntap_get_hwaddr();
	public native int tuntap_set_hwaddr(String hwaddr);
	public native int tuntap_set_descr(String descr);
	public native String tuntap_get_descr();
	public native int tuntap_up();
	public native int tuntap_down();
	public native int tuntap_get_mtu();
	public native int tuntap_set_mtu(int mtu);
	public native int tuntap_set_ip(String ipaddr,int mask);
	public native byte[] tuntap_read(int len);
	public native int tuntap_write(byte[] data,int len);
	public native int tuntap_get_readable();
	public native int tuntap_set_nonblocking(int set);
	public native int tuntap_set_debug(int set);
	public native int tuntap_get_fd();
	private static String osName = System.getProperties().getProperty("os.name");
	static {
		String ver = System.getProperty("sun.arch.data.model");//32 or 64
		
		File file=new File("lib"+ver);//lib32 lib64
		String path=file.getAbsolutePath();
		System.out.println(path);
		
		if(osName.indexOf("Windows") != -1||osName.indexOf("windows") != -1) {//windows
			//System.load(path+"\\TunTap.dll");
			System.load(path+"\\TunTapJNI.dll");
		}else {//linux
			System.load(path+"/TunTap.so");
		}
	}
	public TunTap() {
		tuntap_init();
		tuntap_start(0x0001,257);
	}

}
