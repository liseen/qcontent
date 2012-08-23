%define         qcontent_prefix     /opt/qcontent
%define         qcontent_user       nobody
%define         qcontent_group      users

Name:		qlibvdom
Version:	1.0.0
Release:	1
Summary:	qlibvdom libraray

Group:		Development/Libaries
License:	BSD
Source0:	qlibvdom-%{version}.tar.gz
BuildRoot:	%{build_root}

AutoReqProv:    no
#BuildRequires:	gcc-c++ >= 3.0
#protobuf
#Requires:	protobuf

%description
Qunar vdom c++ library


%prep
%setup -q -n qlibvdom-%{version}

%build
/usr/local/Trolltech/Qt-4.6.2/bin/qmake qlibvdom.pro
make

%install
rm -rf $RPM_BUILD_ROOT
make install INSTALL_ROOT=$RPM_BUILD_ROOT

rm -f `find $RPM_BUILD_ROOT -type f -name '*.debug'`

cur_dir=`pwd`
cd $RPM_BUILD_ROOT && (find . -type f | sed -e 's|^./|/|g' > $cur_dir/%{name}.manifest)
cd $RPM_BUILD_ROOT && (find . -type l | sed -e 's|^./|/|g' >> $cur_dir/%{name}.manifest)

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{name}.manifest
%defattr(-,%{vdom_user},%{vdom_group},-)
#%doc

%changelog
