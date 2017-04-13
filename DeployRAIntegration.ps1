################################################################################
# Custom Variables:

$IntegrationDLLSource = "RA_Integration/RA_Integration.dll"

$VersionDoc = "..\web\LatestIntegration.html"

$ExpectedTag = "RAIntegration"

$ForceUpdate = $true


################################################################################
# Set working directory:

$dir = Split-Path $MyInvocation.MyCommand.Path
Set-Location $dir


################################################################################
# Global Variables:

$Password = ConvertTo-SecureString 'Unused' -AsPlainText -Force
$Credential = New-Object System.Management.Automation.PSCredential ('ec2-user', $Password)
$KeyPath = ".\RetroAchievementsAmazonPrivKey.ppk"
$TargetURL = "RetroAchievements.org"
$WebRoot = "/var/www/html"
$WebRootBin = "/var/www/html/bin"


################################################################################
# Test we are ready for release
$latestTag = git describe --tags --match "$($ExpectedTag).*"
$diffs = git diff HEAD
if( -not $ForceUpdate )
{
    if( ![string]::IsNullOrEmpty( $diffs ) )
    {
        throw "Changes exist, cannot deploy!"
    }
}

$newHTMLContent = "0." + $latestTag.Substring( $ExpectedTag.Length + 1, 3 )
$currentVersion = Get-Content $VersionDoc

if( -not $ForceUpdate )
{
    if( $newHTMLContent -eq $currentVersion )
    {
        throw "We are already on version $currentVersion, nothing to do!"
    }
}

Set-Content $VersionDoc $newHTMLContent


################################################################################
# Upload

# Establish the SFTP connection
$session = ( New-SFTPSession -ComputerName $TargetURL -Credential $Credential -KeyFile $KeyPath )

# Upload the new version html
Set-SFTPFile -SessionId $session.SessionId -LocalFile $VersionDoc -RemotePath $WebRoot -Overwrite

# Upload the zip to the SFTP bin path
Set-SFTPFile -SessionId $session.SessionId -LocalFile $IntegrationDLLSource -RemotePath $WebRootBin -Overwrite

# Disconnect SFTP session
$session.Disconnect()