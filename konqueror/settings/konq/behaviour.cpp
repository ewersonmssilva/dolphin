/**
 *  Copyright (c) 2001 David Faure <david@mandrakesoft.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

// Behaviour options for konqueror

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qwhatsthis.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qvbox.h>
#include <qradiobutton.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <konq_defaults.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kurlrequester.h>
#include <dcopclient.h>
#include <kio/uiserver_stub.h>

#include "behaviour.h"

KBehaviourOptions::KBehaviourOptions(KConfig *config, QString group, QWidget *parent, const char * )
    : KCModule(parent, "kcmkonq"), g_pConfig(config), groupname(group)
{
    QLabel * label;

    QVBoxLayout *lay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

    kfmclientConfig = new KConfig(QString::fromLatin1("kfmclientrc"), false, false);
    kfmclientConfig->setGroup(QString::fromLatin1("Settings")); //use these to set the one-process option in kfmclient

	QVGroupBox * miscGb = new QVGroupBox(i18n("Misc Options"), this);
	lay->addWidget( miscGb );
	QHBox *hbox = new QHBox(miscGb);
	QVBox *vbox = new QVBox(hbox);

	// ----

	winPixmap = new QLabel(hbox);
    winPixmap->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    winPixmap->setPixmap(QPixmap(locate("data",
                                        "kcontrol/pics/onlyone.png")));
    winPixmap->setFixedSize( winPixmap->sizeHint() );


   // ----

    cbNewWin = new QCheckBox(i18n("&Open directories in separate windows"), vbox);
    QWhatsThis::add( cbNewWin, i18n("If this option is checked, Konqueror will open a new window when "
                                    "you open a directory, rather than showing that directory's contents in the current window."));
    connect(cbNewWin, SIGNAL(clicked()), this, SLOT(changed()));
    connect(cbNewWin, SIGNAL(toggled(bool)), SLOT(updateWinPixmap(bool)));

    // ----

    cbListProgress = new QCheckBox( i18n( "&Show network operations in a single window" ), vbox );
    connect(cbListProgress, SIGNAL(clicked()), this, SLOT(changed()));

    QWhatsThis::add( cbListProgress, i18n("Checking this option will group the"
                                          " progress information for all network file transfers into a single window"
                                          " with a list. When the option is not checked, all transfers appear in a"
                                          " separate window.") );


    // --

    cbShowTips = new QCheckBox( i18n( "&Show file tips" ), vbox );
    connect(cbShowTips, SIGNAL(clicked()), this, SLOT(changed()));

    QWhatsThis::add( cbShowTips, i18n("Here you can control if, when moving the mouse over a file, you want to see a "
                                    "small popup window with additional information about that file"));

/*
    connect(cbShowTips, SIGNAL(toggled(bool)), SLOT(slotShowTips(bool)));
    //connect(cbShowTips, SIGNAL(toggled(bool)), sbToolTip, SLOT(setEnabled(bool)));
    //connect(cbShowTips, SIGNAL(toggled(bool)), fileTips, SLOT(setEnabled(bool)));
    fileTips->setBuddy(sbToolTip);
    QString tipstr = i18n("If you move the mouse over a file, you usually see a small popup window that shows some "
                          "additional information about that file. Here, you can set how many items of information "
                          "are displayed");
    QWhatsThis::add( fileTips, tipstr );
    QWhatsThis::add( sbToolTip, tipstr );
*/
    cbShowPreviewsInTips = new QCheckBox( i18n( "&Show previews in file tips" ), vbox );
    connect(cbShowPreviewsInTips, SIGNAL(clicked()), this, SLOT(changed()));

    QWhatsThis::add( cbShowPreviewsInTips, i18n("Here you can control if you want the "
                          "popup window to contain a larger preview for the file, when moving the mouse over it."));

	QHBoxLayout *hlay = new QHBoxLayout( lay );

    label = new QLabel(i18n("Home &URL:"), this);
	hlay->addWidget( label );

	homeURL = new KURLRequester(this);
	homeURL->setMode(KFile::Directory);
	homeURL->setCaption(i18n("Select Home Directory"));
	hlay->addWidget( homeURL );
    connect(homeURL, SIGNAL(textChanged(const QString &)), this, SLOT(changed()));
    label->setBuddy(homeURL);

    QString homestr = i18n("This is the URL (e.g. a directory or a web page) where "
                           "Konqueror will jump to when the \"Home\" button is pressed. "
						   "This usually is your home dirctory, symbolized by a 'tilde' (~).");
    QWhatsThis::add( label, homestr );
    QWhatsThis::add( homeURL, homestr );


    QButtonGroup *bg = new QVButtonGroup( i18n("Ask Confirmation For"), this );
    bg->layout()->setSpacing( KDialog::spacingHint() );
    QWhatsThis::add( bg, i18n("This option tells Konqueror whether to ask"
       " for a confirmation when you \"delete\" a file."
       " <ul><li><em>Move To Trash:</em> moves the file to your trash directory,"
       " from where it can be recovered very easily.</li>"
       " <li><em>Delete:</em> simply deletes the file.</li>"
       " <li><em>Shred:</em> not only deletes the file, but overwrites"
       " the area on the disk where the file is stored, making recovery impossible."
       " You should not remove confirmation for this method unless you routinely work"
       " with very confidential information.</li></ul>") );

    connect(bg, SIGNAL( clicked( int ) ), SLOT( changed() ));

    cbMoveToTrash = new QCheckBox( i18n("Move to trash"), bg );

    cbDelete = new QCheckBox( i18n("Delete"), bg );

    cbShred = new QCheckBox( i18n("Shred"), bg );

    lay->addWidget(bg);


    QString opstrg = i18n("With this option activated, only one instance of Konqueror "
                          "will exist in the memory of your computer at any moment, "
                          "no matter how many browsing windows you open, "
                          "thus reducing resource requirements."
                          "<p>Be aware that this also means that, if something goes wrong, "
                          "all your browsing windows will be closed simultaneously.");
    QString opstrl = i18n("With this option activated, only one instance of Konqueror "
                          "will exist in the memory of your computer at any moment, "
                          "no matter how many local browsing windows you open, "
                          "thus reducing resource requirements."
                          "<p>Be aware that this also means that, if something goes wrong, "
                          "all your local browsing windows will be closed simultaneously");
    QString opstrw = i18n("With this option activated, only one instance of Konqueror "
                          "will exist in the memory of your computer at any moment, "
                          "no matter how many web browsing windows you open, "
                          "thus reducing resource requirements."
                          "<p>Be aware that this also means that, if something goes wrong, "
                          "all your web browsing windows will be closed simultaneously");

    bgOneProcess = new QVButtonGroup(i18n("Minimize Memory Usage"), this);
    bgOneProcess->setExclusive( true );
    connect(bgOneProcess, SIGNAL(clicked(int)), this, SLOT(changed()));
    {
        rbOPNever = new QRadioButton(i18n("&Never"), bgOneProcess);
        QWhatsThis::add( rbOPNever, i18n("Disables the minimization of memory usage and allows you "
                                         "to make each browsing activity independent from the others"));

        rbOPLocal = new QRadioButton(i18n("For &local browsing only (recommended)"), bgOneProcess);
        QWhatsThis::add( rbOPLocal, opstrl);

        rbOPWeb = new QRadioButton(i18n("For &web browsing only"), bgOneProcess);
        QWhatsThis::add( rbOPWeb, opstrw);

        rbOPAlways = new QRadioButton(i18n("Alwa&ys (use with care)"), bgOneProcess);
        QWhatsThis::add( rbOPAlways, opstrg);

        rbOPLocal->setChecked(true);
    }

	lay->addWidget( bgOneProcess );
	lay->addStretch();

    load();
}

void KBehaviourOptions::slotShowTips(bool b)
{
//    sbToolTip->setEnabled( b );
    cbShowPreviewsInTips->setEnabled( b );
//    fileTips->setEnabled( b );
	
}

void KBehaviourOptions::load()
{
    g_pConfig->setGroup( groupname );
    cbNewWin->setChecked( g_pConfig->readBoolEntry("AlwaysNewWin", false) );
    updateWinPixmap(cbNewWin->isChecked());

    homeURL->setURL(g_pConfig->readEntry("HomeURL", "~"));

    bool stips = g_pConfig->readBoolEntry( "ShowFileTips", true );
    cbShowTips->setChecked( stips );
    slotShowTips( stips );

    bool showPreviewsIntips = g_pConfig->readBoolEntry( "ShowPreviewsInFileTips", true );
    cbShowPreviewsInTips->setChecked( showPreviewsIntips );

//    if (!stips) sbToolTip->setEnabled( false );
    if (!stips) cbShowPreviewsInTips->setEnabled( false );

//    sbToolTip->setValue( g_pConfig->readNumEntry( "FileTipItems", 6 ) );

    QString val = kfmclientConfig->readEntry( QString::fromLatin1("StartNewKonqueror"),
                                              QString::fromLatin1("Web only") );
    if (val == QString::fromLatin1("Web only"))
        rbOPLocal->setChecked(true);
    else if (val == QString::fromLatin1("Local only"))
        rbOPWeb->setChecked(true);
    else if (val == QString::fromLatin1("Always") ||
             val == QString::fromLatin1("true") ||
             val == QString::fromLatin1("TRUE") ||
             val == QString::fromLatin1("1"))
        rbOPNever->setChecked(true);
    else
        rbOPAlways->setChecked(true);

    KConfig config("uiserverrc");
    config.setGroup( "UIServer" );

    cbListProgress->setChecked( config.readBoolEntry( "ShowList", false ) );
    
    g_pConfig->setGroup( "Trash" );
    cbMoveToTrash->setChecked( g_pConfig->readBoolEntry("ConfirmTrash", DEFAULT_CONFIRMTRASH) );
    cbDelete->setChecked( g_pConfig->readBoolEntry("ConfirmDelete", DEFAULT_CONFIRMDELETE) );
    cbShred->setChecked( g_pConfig->readBoolEntry("ConfirmShred", DEFAULT_CONFIRMSHRED) );
}

void KBehaviourOptions::defaults()
{
    cbNewWin->setChecked(false);

    homeURL->setURL("~");

    rbOPLocal->setChecked(true);

    cbListProgress->setChecked( false );

    cbShowTips->setChecked( true );
    //sbToolTip->setEnabled( true );
    //sbToolTip->setValue( 6 );

    cbShowPreviewsInTips->setChecked( true );
    cbShowPreviewsInTips->setEnabled( true );

    cbMoveToTrash->setChecked( DEFAULT_CONFIRMTRASH );
    cbDelete->setChecked( DEFAULT_CONFIRMDELETE );
    cbShred->setChecked( DEFAULT_CONFIRMSHRED );
}

void KBehaviourOptions::save()
{
    g_pConfig->setGroup( groupname );

    g_pConfig->writeEntry( "AlwaysNewWin", cbNewWin->isChecked() );
    g_pConfig->writeEntry( "HomeURL", homeURL->url().isEmpty()? "~" : homeURL->url() );

    g_pConfig->writeEntry( "ShowFileTips", cbShowTips->isChecked() );
    g_pConfig->writeEntry( "ShowPreviewsInFileTips", cbShowPreviewsInTips->isChecked() );
//    g_pConfig->writeEntry( "FileTipsItems", sbToolTip->value() );

    QString val = QString::fromLatin1("Web only");
    if (rbOPWeb->isChecked())
        val = QString::fromLatin1("Local only");
    else if (rbOPNever->isChecked())
        val = QString::fromLatin1("Always");
    else if (rbOPAlways->isChecked())
        val = QString::fromLatin1("Never");

    g_pConfig->setGroup( "Trash" );
    g_pConfig->writeEntry( "ConfirmTrash", cbMoveToTrash->isChecked());
    g_pConfig->writeEntry( "ConfirmDelete", cbDelete->isChecked());
    g_pConfig->writeEntry( "ConfirmShred", cbShred->isChecked());
    g_pConfig->sync();
    
    kfmclientConfig->writeEntry(QString::fromLatin1("StartNewKonqueror"), val);
    kfmclientConfig->sync();

    // UIServer setting
    KConfig config("uiserverrc");
    config.setGroup( "UIServer" );
    config.writeEntry( "ShowList", cbListProgress->isChecked() );
    config.sync();
    // Tell the running server
    if ( kapp->dcopClient()->isApplicationRegistered( "kio_uiserver" ) )
    {
      UIServer_stub uiserver( "kio_uiserver", "UIServer" );
      uiserver.setListMode( cbListProgress->isChecked() );
    }

    // Send signal to konqueror
    // Warning. In case something is added/changed here, keep kfmclient in sync
    QByteArray data;
    if ( !kapp->dcopClient()->isAttached() )
      kapp->dcopClient()->attach();
    kapp->dcopClient()->send( "konqueror*", "KonquerorIface", "reparseConfiguration()", data );
}

void KBehaviourOptions::updateWinPixmap(bool b)
{
  if (b)
    winPixmap->setPixmap(QPixmap(locate("data",
                                        "kcontrol/pics/overlapping.png")));
  else
    winPixmap->setPixmap(QPixmap(locate("data",
                                        "kcontrol/pics/onlyone.png")));
}

QString KBehaviourOptions::quickHelp() const
{
    return i18n("<h1>Trash Options</h1> Here you can modify the behavior "
                "of Konqueror when you want to delete a file."
                "<h2>On delete:</h2>This option determines what Konqueror "
                "will do with a file you chose to delete (e.g. in a context menu).<ul>"
                "<li><em>Move To Trash</em> will move the file to the trash directory, "
                "instead of deleting it, so you can easily recover it.</li>"
                "<li><em>Delete</em> will simply delete the file.</li>"
                "<li><em>Shred</em> will not only delete the file, but will first "
                "overwrite it with different bit patterns. This makes recovery impossible. "
                "Use it, if you're keeping very sensitive data."
                "<h2>Confirm destructive actions</h2>Check this box if you want Konqueror "
                "to ask \"Are you sure?\" before doing any destructive action (e.g. delete or shred).");
}

void KBehaviourOptions::changed()
{
  emit KCModule::changed(true);
}

#include "behaviour.moc"
