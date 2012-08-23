%define         qc_prefix     /opt/qcontent
%define         qc_user       nobody
%define         qc_group      nobody

Name:		qc-qcontent

Version:	1.0.0
Release:	1
Summary:	qcontent

Group:		QContent
License:	BSD
URL:		http://svn.corp.qunar.com/svn/search-team/qcontent-deps/package

Source0:	qcontent-%{version}.tar.gz

BuildRoot:	%{build_root}

AutoReqProv:    no
BuildRequires:	gcc >= 3.0, qc-libev, pcre-devel, qc-qt, qc-msgpack-rpc-cpp, qc-cityhash, qc-glog, qc-google-perftools,  qc-icu4c, qc-tokyotyrant, qc-jsoncpp, xorg-x11-server-Xvfb

Requires:	    qc-libev, pcre, qc-qt, qc-msgpack-rpc-cpp, qc-cityhash, qc-glog, qc-google-perftools,  qc-icu4c, qc-tokyotyrant, qc-jsoncpp, xorg-x11-server-Xvfb

%description
qcontent

%prep
#%setup -q -n qcontent-%{version}
%setup -q -n qcontent

%build

/opt/qt/bin/qmake
make %{?_smp_mflags}

%install

rm -rf $RPM_BUILD_ROOT
make install INSTALL_ROOT=$RPM_BUILD_ROOT

#find $RPM_BUILD_ROOT -depth -type d -exec rmdir {} 2>/dev/null \;

cur_dir=`pwd`
cd $RPM_BUILD_ROOT && (find . -type f | sed -e 's|^./|/|g' > $cur_dir/%{name}.manifest)
cd $RPM_BUILD_ROOT && (find . -type l | sed -e 's|^./|/|g' >> $cur_dir/%{name}.manifest)

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{name}.manifest
%defattr(-,%{qc_user},%{qc_group},-)
#%doc

%changelog
* Thu Sep 15 2011 xunxin <xunxin.wan@qunar.com>
- initial packaging

