-- phpMyAdmin SQL Dump
-- version 3.5.8.1
-- http://www.phpmyadmin.net
--
-- Host: 10.246.16.8:3306
-- Generation Time: Aug 21, 2015 at 11:02 AM
-- Server version: 5.5.44-MariaDB-1~wheezy
-- PHP Version: 5.3.3-7+squeeze15

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

--
-- Database: `pistol_wtf`
--

-- --------------------------------------------------------

--
-- Table structure for table `tbl_clients`
--

CREATE TABLE IF NOT EXISTS `tbl_clients` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `client` varchar(64) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `id` (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=8 ;

-- --------------------------------------------------------

--
-- Table structure for table `tbl_symbols`
--

CREATE TABLE IF NOT EXISTS `tbl_symbols` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `symbol` varchar(64) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `id` (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=7 ;

-- --------------------------------------------------------

--
-- Table structure for table `tbl_values`
--

CREATE TABLE IF NOT EXISTS `tbl_values` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `clientId` int(11) NOT NULL,
  `symbolId` int(11) NOT NULL,
  `value` int(11) NOT NULL,
  `timeStamp` bigint(20) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `clientId` (`clientId`,`symbolId`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=8 ;
