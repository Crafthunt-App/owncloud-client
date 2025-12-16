/*
 * Copyright (C) Lisa Reese <lisa.reese@kiteworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */
#include "urlpagecontroller.h"

#include "accessmanager.h"
#include "networkadapters/determineauthtypeadapter.h"
#include "networkadapters/discoverwebfingerserviceadapter.h"
#include "networkadapters/resolveurladapter.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <QWizardPage>

namespace OCC {

UrlPageController::UrlPageController(QWizardPage *page, AccessManager *accessManager, QObject *parent)
    : QObject(parent)
    , _page(page)
    , _accessManager(accessManager)
{
    buildPage();

    QString themeUrl = Theme::instance()->overrideServerUrlV2();
    if (_urlField && !themeUrl.isEmpty()) {
        setUrl(themeUrl);
        // The theme provides the url, don't let the user change it!
        _urlField->setEnabled(false);
        _instructionLabel->setText(tr("Your web browser will be opened to complete sign in."));
    }
}

void UrlPageController::buildPage()
{
    if (!_page)
        return;

    QString appName = Theme::instance()->appNameGUI();

    // Logo with elevated styling
    QLabel *logoLabel = new QLabel({}, _page);
    QIcon headerIcon = Theme::instance()->wizardHeaderLogo();
    if (!headerIcon.isNull()) {
        // Use a reasonable size for the logo
        QPixmap logoPix = headerIcon.pixmap(QSize(200, 100));
        logoLabel->setPixmap(logoPix);
        logoLabel->setMinimumSize(220, 120);
        logoLabel->setMaximumSize(280, 140);
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    logoLabel->setAccessibleName(tr("%1 logo").arg(appName));
    logoLabel->setScaledContents(false);
    logoLabel->setStyleSheet(
        "QLabel { "
        "  padding: 16px; "
        "  background-color: white; "
        "  border-radius: 16px; "
        "}"
    );

    // Bold, striking welcome label with BauGPT orange accent
    QLabel *welcomeLabel = new QLabel(tr("Welcome to %1").arg(appName), _page);
    QFont welcomeFont = welcomeLabel->font();
    welcomeFont.setWeight(QFont::Black);
    welcomeFont.setPixelSize(32);
    welcomeFont.setLetterSpacing(QFont::AbsoluteSpacing, -0.5);
    welcomeLabel->setFont(welcomeFont);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPalette welcomePalette = welcomeLabel->palette();
    welcomePalette.setColor(QPalette::WindowText, QColor("#2C3E50"));
    welcomeLabel->setPalette(welcomePalette);
    welcomeLabel->setStyleSheet(
        "QLabel { "
        "  color: #2C3E50; "
        "  background: transparent; "
        "  padding: 8px 0; "
        "}"
    );

    // Refined instruction text
    _instructionLabel = new QLabel(tr("Enter your server address to get started. Your web browser will be opened to complete sign in."), _page);
    QFont font = _instructionLabel->font();
    font.setPixelSize(15);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0.2);
    _instructionLabel->setFont(font);
    _instructionLabel->setWordWrap(true);
    _instructionLabel->setAlignment(Qt::AlignCenter);
    _instructionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPalette instructionPalette = _instructionLabel->palette();
    instructionPalette.setColor(QPalette::WindowText, QColor("#5D6D7E"));
    _instructionLabel->setPalette(instructionPalette);
    _instructionLabel->setStyleSheet(
        "QLabel { "
        "  color: #5D6D7E; "
        "  background: transparent; "
        "  line-height: 1.6; "
        "  padding: 0 24px; "
        "}"
    );

    // Modern, elevated input field with BauGPT orange focus accent
    _urlField = new QLineEdit(_page);
    _urlField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _urlField->setMinimumHeight(52);
    _urlField->setMinimumWidth(480);
    _urlField->setMaximumWidth(520);
    _urlField->setPlaceholderText(Theme::instance()->wizardUrlPlaceholder().isEmpty()
        ? tr("https://your-server.com")
        : Theme::instance()->wizardUrlPlaceholder());
    _urlField->setFocusPolicy(Qt::StrongFocus);
    _urlField->setAccessibleName(tr("Server address field"));
    _urlField->setAccessibleDescription(tr("Enter your server address here"));
    _urlField->setObjectName("ServerAddressLineEdit");

    QFont urlFont = _urlField->font();
    urlFont.setPixelSize(15);
    _urlField->setFont(urlFont);

    // Set palette explicitly for light mode appearance
    QPalette urlPalette = _urlField->palette();
    urlPalette.setColor(QPalette::Base, QColor("#FFFFFF"));
    urlPalette.setColor(QPalette::Text, QColor("#2C3E50"));
    urlPalette.setColor(QPalette::PlaceholderText, QColor("#95A5B8"));
    _urlField->setPalette(urlPalette);

    _urlField->setStyleSheet(
        "QLineEdit { "
        "  background-color: white; "
        "  border: 2px solid #E8EAED; "
        "  border-radius: 12px; "
        "  padding: 14px 20px; "
        "  color: #2C3E50; "
        "  font-size: 15px; "
        "} "
        "QLineEdit:hover { "
        "  border-color: #FF6B35; "
        "  background-color: #FFFFFF; "
        "} "
        "QLineEdit:focus { "
        "  border: 3px solid #FF6B35; "
        "  background-color: #FFFFFF; "
        "  padding: 13px 19px; "
        "} "
    );

    // Modern error field with refined styling
    _errorField = new QLabel(QString(), _page);
    QFont errorFont = _errorField->font();
    errorFont.setPixelSize(13);
    errorFont.setWeight(QFont::Medium);
    _errorField->setFont(errorFont);
    _errorField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _errorField->setWordWrap(true);
    _errorField->setAlignment(Qt::AlignCenter);
    _errorField->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    _errorField->setMinimumWidth(480);
    _errorField->setMaximumWidth(520);
    _errorField->setVisible(false); // Hide by default, show only when there's an error

    QPalette errorPalette = _errorField->palette();
    errorPalette.setColor(QPalette::WindowText, QColor("#E74C3C"));
    _errorField->setPalette(errorPalette);

    _errorField->setStyleSheet(
        "QLabel { "
        "  color: #E74C3C; "
        "  background-color: #FADBD8; "
        "  border: 1px solid #F1948A; "
        "  border-radius: 8px; "
        "  padding: 12px 16px; "
        "  margin-top: 8px; "
        "}"
    );

    // Footer logo with subtle styling
    QLabel *footerLogoLabel = nullptr;
    if (!Theme::instance()->wizardFooterLogo().isNull()) {
        footerLogoLabel = new QLabel({}, _page);
        footerLogoLabel->setPixmap(Theme::instance()->wizardFooterLogo().pixmap(120, 60));
        footerLogoLabel->setAlignment(Qt::AlignCenter);
        footerLogoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        footerLogoLabel->setAccessibleName(tr("Additional logo defined by the organization"));
        footerLogoLabel->setStyleSheet(
            "QLabel { "
            "  padding: 8px; "
            "  opacity: 0.7; "
            "}"
        );
    }

    // Create horizontal layout for input field centering
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addStretch(1);
    inputLayout->addWidget(_urlField);
    inputLayout->addStretch(1);

    // Create horizontal layout for error field centering
    QHBoxLayout *errorLayout = new QHBoxLayout();
    errorLayout->addStretch(1);
    errorLayout->addWidget(_errorField);
    errorLayout->addStretch(1);

    // Generous spacing for a refined, modern feel
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(60, 40, 60, 40);
    layout->setSpacing(0);

    layout->addStretch(1);
    layout->addWidget(logoLabel, Qt::AlignCenter);
    layout->addSpacing(32);
    layout->addWidget(welcomeLabel, Qt::AlignCenter);
    layout->addSpacing(12);
    layout->addWidget(_instructionLabel, Qt::AlignCenter);
    layout->addSpacing(28);
    layout->addLayout(inputLayout);
    layout->addLayout(errorLayout);
    layout->addSpacing(20);
    if (footerLogoLabel) {
        layout->addWidget(footerLogoLabel, Qt::AlignCenter);
        layout->addSpacing(8);
    }
    layout->addStretch(1);

    _page->setLayout(layout);

    _urlField->setFocus(Qt::OtherFocusReason);
}

void UrlPageController::setUrl(const QString &urlText)
{
    QSignalBlocker blocker(_urlField);
    _urlField->setText(urlText);
}

void UrlPageController::handleError(const QString &error)
{
    _errorField->setText(error);
    _errorField->setVisible(!error.isEmpty()); // Show error field only when there's an error message
    _results.error = error;
    Q_EMIT failure(_results);
}

QUrl UrlPageController::checkUrl()
{
    if (!_accessManager)
        return QUrl();

    QString userUrl = _urlField->text();

    // we need to check the scheme to be sure it's not there, but using the old "string matching" technique was not great
    // because it added https:// to urls that had valid schemes but which did not match http or https. That was more complicating
    // than useful so now we only add https if there really is no valid scheme to start with.
    // the caller should decide which schemes are allowed and take whatever action it sees fit

    // strict mode really does not appear to be useful wrt catching malformed urls but hey, why not.
    QUrl testUrl(userUrl, QUrl::StrictMode);
    QString scheme = testUrl.scheme();
    if (scheme.isEmpty()) {
        userUrl.prepend(QStringLiteral("https://"));
        testUrl.setScheme(QStringLiteral("https"));
        // so far I don't see testUrl coming back as invalid even if it should be but it seems reasonable to set the scheme anyway before testing validity.
        // we catch the invalid condition later using a new url after reconstructing it with the default scheme added to the input string. see comment below.
        // no I can't explain this behavior - it's a Qt mystery
    }
    if (!testUrl.isValid()) {
        // if it's already dead, stop trying
        return testUrl;
    }

    // we can't recycle the testUrl because for reasons completely unknown to me, if you try to adjust it or set the path it destroys
    // the host so we get https:///kwdav/ every time, even if the testUrl was valid
    // solution = instantiate a new url <shrug>
    QUrl finalUrl(userUrl);

    if (finalUrl.isValid()) {
        finalUrl = finalUrl.adjusted(QUrl::RemoveUserInfo | QUrl::RemoveQuery);
        const QString serverPathOverride = Theme::instance()->overrideServerPath();
        if (!serverPathOverride.isEmpty()) {
            finalUrl.setPath(serverPathOverride);
        }
    }
    return finalUrl;
}

bool UrlPageController::validate()
{
    _results = {};
    _errorField->clear();
    _errorField->setVisible(false); // Hide error field when starting validation

    if (!_accessManager) {
        handleError(QStringLiteral("No valid access manager is available"));
        return false;
    }

    // always clear the access manager data before revalidating the url
    _accessManager->reset();

    QUrl givenUrl = checkUrl();
    if (!givenUrl.isValid()) {
        QString fullError = givenUrl.errorString();
        QStringList parts = fullError.split(QStringLiteral(";"), Qt::SkipEmptyParts);
        handleError(tr("Invalid server URL: %1").arg(parts[0] + parts[1]));
        return false;
    } else if (givenUrl.scheme().isEmpty() || givenUrl.scheme() != QStringLiteral("https")) // scheme should not be empty but who knows
    {
        handleError(tr("Invalid URL scheme. Only https is accepted."));
        return false;
    }

    setUrl(givenUrl.toDisplayString());

    DiscoverWebFingerServiceAdapter webfingerServiceAdapter(_accessManager, givenUrl);
    const DiscoverWebFingerServiceResult webfingerServiceResult = webfingerServiceAdapter.getResult();

    // in case any kind of error occurs, we assume the WebFinger service is not available
    if (webfingerServiceResult.success()) {
        _results.baseServerUrl = givenUrl;
        _results.webfingerServiceUrl = QUrl(webfingerServiceResult.href);
    } else {
        // first, we must resolve the actual server URL
        // note we have to pass the wizard dialog to the adapter to parent the tlsErrorDialog
        ResolveUrlAdapter urlResolver(_accessManager, givenUrl, _page->parentWidget());
        const ResolveUrlResult resolveUrlResult = urlResolver.getResult();

        if (!resolveUrlResult.success()) {
            handleError(resolveUrlResult.error);
            return false;
        }

        const QUrl finalUrl = resolveUrlResult.resolvedUrl;
        // I don't know how likely it is that the resolved url will contain a query but from other code it is
        // evident that url queries are banned. so if it comes back with a query on it, it's a nogo
        if (finalUrl.hasQuery()) {
            QString errorMsg = tr("The requested URL failed with query value: %1").arg(finalUrl.query());
            handleError(errorMsg);
            return false;
        }

        if (!resolveUrlResult.acceptedCertificates.isEmpty()) {
            // future requests made through this access manager need to include any certificates
            _accessManager->addCustomTrustedCaCertificates(resolveUrlResult.acceptedCertificates);
            // save/return this also for the account setup, as the account maintains the set of certificates, too.
            _results.certificates = resolveUrlResult.acceptedCertificates;
        }

        // This is now a mere formality to be very very very sure that the base url uses oauth. In the unlikely event that it doesn't
        // the job simply fails.
        DetermineAuthTypeAdapter authTypeAdapter(_accessManager, finalUrl);
        const DetermineAuthTypeResult authResult = authTypeAdapter.getResult();
        if (!authResult.success()) {
            handleError(authResult.error);
            return false;
        }

        Q_ASSERT(authResult.type == AuthenticationType::OAuth);
        _results.baseServerUrl = finalUrl;
    }
    Q_EMIT success(_results);
    return true;
}

}
